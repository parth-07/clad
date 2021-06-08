#include "clad/Differentiator/DiffPlanner.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/TemplateDeduction.h"

#include "llvm/Support/SaveAndRestore.h"

#include "clad/Differentiator/Compatibility.h"

using namespace clang;

namespace clad {
  static SourceLocation noLoc;
  
  // Returns DeclRefExpr of function argument of differentiation calls.
  // If argument is passed for relevantAncestor, then it's value will be 
  // modified in-place to contain relevant ancestor of the function 
  // argument's DeclRefExpr
  // Here relevant ancestor is nearest ancestor of ImplicitCastExpr or 
  // UnaryOperator type.
  DeclRefExpr* getArgFunction(CallExpr* call,
                              Sema& SemaRef,
                              Expr** relevantAncestor = nullptr) {
    struct Finder :
      RecursiveASTVisitor<Finder> {
        DeclRefExpr* m_FnDRE = nullptr;
        Expr** m_RelevantAncestor = nullptr;
        Sema& m_SemaRef;
        Finder(Sema& SemaRef, Expr** relevantAncestor=nullptr) : m_SemaRef(SemaRef), m_RelevantAncestor(relevantAncestor) {}
        bool VisitExpr(Expr* E) {
          if (m_RelevantAncestor) {
            switch (E->getStmtClass()) {
              case Stmt::StmtClass::ImplicitCastExprClass: LLVM_FALLTHROUGH;
              case Stmt::StmtClass::UnaryOperatorClass:
                *m_RelevantAncestor = E;
                break; 
            }
          }
          if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
            if (auto VD = dyn_cast<VarDecl>(DRE->getDecl())) {
              auto varType = VD->getType().getTypePtr();
              if (varType->isStructureOrClassType()) {
                auto RD = varType->getAsCXXRecordDecl();
                // TODO: Use namelookup instead to find operator() method
                for (auto methodIter = RD->method_begin(), E = RD->method_end(); 
                     methodIter != E; 
                     ++methodIter) {
                  if (methodIter->getNameAsString() == "operator()") {
                    CXXScopeSpec CSS;
                    CSS.Extend(m_SemaRef.getASTContext(), RD->getIdentifier(), 
                               noLoc, noLoc);
                    m_FnDRE = m_SemaRef.BuildDeclRefExpr(*methodIter,
                                  methodIter->getType(), 
                                  ExprValueKind::VK_RValue,
                                  noLoc, &CSS);
                    break;                                  
                  }
                }
              }
              else {
                TraverseStmt(VD->getInit());
              }
            }
            else if (auto FD = dyn_cast<FunctionDecl>(DRE->getDecl()))
              m_FnDRE = DRE;
            return false;  
          }
          return true; 
        }
      } finder(SemaRef, relevantAncestor);

      assert(cast<NamespaceDecl>(call->getDirectCallee()->
             getDeclContext())->getName() == "clad" &&
             "Should be called for clad:: special functions!");

      finder.TraverseStmt(call->getArg(0));
      return finder.m_FnDRE;
  }

  void DiffRequest::updateCall(FunctionDecl* FD, Sema& SemaRef) {
    CallExpr* call = this->CallContext;
    // Index of "code" parameter:
    auto codeArgIdx = static_cast<int>(call->getNumArgs()) - 1;
    assert(call && "Must be set");
    assert(FD && "Trying to update with null FunctionDecl");

    Expr* oldArgDREParent = nullptr;
    DeclRefExpr* oldDRE = getArgFunction(call, SemaRef, &oldArgDREParent);
    if (!oldDRE)
      llvm_unreachable("Trying to differentiate something unsupported");

    ASTContext& C = SemaRef.getASTContext();
    // Create ref to generated FD.
    Expr* DRE = DeclRefExpr::Create(C, oldDRE->getQualifierLoc(), noLoc,
                                    FD, false, FD->getNameInfo(), FD->getType(),
                                    oldDRE->getValueKind());
    // FIXME: I am not sure if the following part is necessary:
    // using call->setArg(0, DRE) seems to be sufficient,
    // though the real AST allways contains the ImplicitCastExpr (function ->
    // function ptr cast) or UnaryOp (method ptr call).
    if (auto oldCast = dyn_cast<ImplicitCastExpr>(oldArgDREParent)) {
      // Cast function to function pointer.
      auto newCast = ImplicitCastExpr::Create(C,
                                              C.getPointerType(FD->getType()),
                                              oldCast->getCastKind(),
                                              DRE,
                                              nullptr,
                                              oldCast->getValueKind()
                                              CLAD_COMPAT_CLANG12_CastExpr_GetFPO(oldCast));
      call->setArg(0, newCast);
    }
    else if (auto oldUnOp = dyn_cast<UnaryOperator>(oldArgDREParent)) {
      // Add the "&" operator
      auto newUnOp = SemaRef.BuildUnaryOp(nullptr,
                                          noLoc,
                                          oldUnOp->getOpcode(),
                                          DRE).get();
      call->setArg(0, newUnOp);
    }
    else
      llvm_unreachable("Trying to differentiate something unsupported");

    // Update the code parameter.
    if (CXXDefaultArgExpr* Arg
        = dyn_cast<CXXDefaultArgExpr>(call->getArg(codeArgIdx))) {
      clang::LangOptions LangOpts;
      LangOpts.CPlusPlus = true;
      clang::PrintingPolicy Policy(LangOpts);

      std::string s;
      llvm::raw_string_ostream Out(s);
      FD->print(Out, Policy);
      Out.flush();

      // Copied and adapted from clang::Sema::ActOnStringLiteral.
      QualType CharTyConst = C.CharTy;
      CharTyConst.addConst();
      // Get an array type for the string, according to C99 6.4.5. This includes
      // the nul terminator character as well as the string length for pascal
      // strings.
      QualType StrTy =
        clad_compat::getConstantArrayType(C, CharTyConst,
                               llvm::APInt(32, Out.str().size() + 1),
                               nullptr,
                               ArrayType::Normal,
                               /*IndexTypeQuals*/0);

      StringLiteral* SL =
        StringLiteral::Create(C,
                              Out.str(),
                              StringLiteral::Ascii,
                              /*Pascal*/false,
                              StrTy,
                              noLoc);
      Expr* newArg =
        SemaRef.ImpCastExprToType(SL,
                                  Arg->getType(),
                                  CK_ArrayToPointerDecay).get();
      call->setArg(codeArgIdx, newArg);
    }

    // Replace old specialization of clad::gradient with a new one that matches
    // the type of new argument.

    auto CladGradientFDeclOld = call->getDirectCallee();
    auto CladGradientExprOld = call->getCallee();
    auto CladGradientFTemplate = CladGradientFDeclOld->getPrimaryTemplate();

    FunctionDecl* CladGradientFDeclNew = nullptr;
    sema::TemplateDeductionInfo Info(noLoc);
    // Create/get template specialization of clad::gradient that matches
    // argument types. Result is stored to CladGradientFDeclNew.
    SemaRef.DeduceTemplateArguments(CladGradientFTemplate,
                                    /* ExplicitTemplateArgs */ nullptr,
                                    /* Args */
                                    llvm::ArrayRef<Expr*>(call->getArgs(),
                                                          call->getNumArgs()),
                                    /* Specialization */ CladGradientFDeclNew,
                                    Info,
                                    /* PartialOverloading */ false,
                                    /* CheckNonDependent */
                                    [] (llvm::ArrayRef<QualType>) {
                                      return false;
                                    });
    // DeclRefExpr for new specialization.
    Expr* CladGradientExprNew = clad_compat::GetResult<Expr*>(
      SemaRef.BuildDeclRefExpr(CladGradientFDeclNew,
                               CladGradientFDeclNew->getType(),
                               CladGradientExprOld->getValueKind(),
                               CladGradientExprOld->getEndLoc())
    );
    // Add function to pointer cast.
    CladGradientExprNew = clad_compat::GetResult<Expr*>(
      SemaRef.CallExprUnaryConversions(CladGradientExprNew)
    );

    // Replace the old clad::gradient by the new one.
    call->setCallee(CladGradientExprNew);
  }

  DiffCollector::DiffCollector(DeclGroupRef DGR, DiffInterval& Interval,
                               const DerivativesSet& Derivatives,
                               DiffSchedule& plans, clang::Sema& S)
    : m_Interval(Interval), m_GeneratedDerivatives(Derivatives),
      m_DiffPlans(plans), m_TopMostFD(nullptr), m_Sema(S) {

    if (Interval.empty())
      return;


    for (Decl* D : DGR) {
      // Skip over the derivatives that we produce.
      if (m_GeneratedDerivatives.count(D))
        continue;
      TraverseDecl(D);
    }
  }

  bool DiffCollector::isInInterval(SourceLocation Loc) const {
    const SourceManager &SM = m_Sema.getSourceManager();
    for (size_t i = 0, e = m_Interval.size(); i < e; ++i) {
      SourceLocation B = m_Interval[i].getBegin();
      SourceLocation E = m_Interval[i].getEnd();
      assert((i == e-1 || E.isValid()) && "Unexpected open interval");
      assert(E.isInvalid() || SM.isBeforeInTranslationUnit(B, E));
      if (E.isValid() &&
          clad_compat::SourceManager_isPointWithin(SM, Loc, B, E))
        return true;
      else if (SM.isBeforeInTranslationUnit(B, Loc))
        return true;
    }
    return false;
  }

  bool DiffCollector::VisitCallExpr(CallExpr* E) {
    // Check if we should look into this.
    if (!isInInterval(E->getEndLoc()))
        return true;

    FunctionDecl* FD = E->getDirectCallee();
    if (!FD)
      return true;
    // We need to find our 'special' diff annotated such:
    // clad::differentiate(...) __attribute__((annotate("D")))
    // TODO: why not check for its name? clad::differentiate/gradient?
    const AnnotateAttr* A = FD->getAttr<AnnotateAttr>();
    if (A && (A->getAnnotation().equals("D") || A->getAnnotation().equals("G") 
        || A->getAnnotation().equals("H") || A->getAnnotation().equals("J"))) {
      // A call to clad::differentiate or clad::gradient was found.
      DeclRefExpr* DRE = getArgFunction(E, m_Sema);
      if (!DRE)
        return true;
      DiffRequest request{};

      if (A->getAnnotation().equals("D")) {
        request.Mode = DiffMode::forward;
        llvm::APSInt derivativeOrderAPSInt
          = FD->getTemplateSpecializationArgs()->get(0).getAsIntegral();
        // We know the first template spec argument is of unsigned type
        assert(derivativeOrderAPSInt.isUnsigned() && "Must be unsigned");
        unsigned derivativeOrder = derivativeOrderAPSInt.getZExtValue();
        request.RequestedDerivativeOrder = derivativeOrder;
      } else if (A->getAnnotation().equals("H")) {
        request.Mode = DiffMode::hessian;
      } else if (A->getAnnotation().equals("J")) {
        request.Mode = DiffMode::jacobian;
      } else {
        request.Mode = DiffMode::reverse;
      }
      request.CallContext = E;
      request.CallUpdateRequired = true;
      request.VerboseDiags = true;
      request.Args = E->getArg(1);
      auto derivedFD = cast<FunctionDecl>(DRE->getDecl());
      request.Function = derivedFD;
      request.BaseFunctionName = derivedFD->getNameAsString();

      // FIXME: add support for nested calls to clad::differentiate/gradient
      // inside differentiated functions
      assert(!m_TopMostFD &&
             "nested clad::differentiate/gradient are not yet supported");
      llvm::SaveAndRestore<const FunctionDecl*> saveTopMost = m_TopMostFD;
      m_TopMostFD = FD;
      TraverseDecl(derivedFD);
      m_DiffPlans.push_back(std::move(request));
    }
    /*else if (m_TopMostFD) {
      // If another function is called inside differentiated function,
      // this will be handled by Forward/ReverseModeVisitor::Derive.
    }*/
    return true;     // return false to abort visiting.
  }
} // end namespace
