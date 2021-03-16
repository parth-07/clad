#include "clad/Differentiator/DiffPlanner.h"

#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/TemplateDeduction.h"

#include "llvm/Support/SaveAndRestore.h"

#include "clad/Differentiator/Compatibility.h"

using namespace clang;

namespace clad {
  static SourceLocation noLoc;

  // Returns address of parent of Expr node obtained 
  // by skipping past any parantheses, implicit casts
  // and UnaryOperators from Expr pointed by `E`.
  // If considerParanAsParent is false,then returns last non-parantheses
  // ancestor of Expr node obtained by skipping.
  // Returns nullptr, if there is no skipping of nodes
  // from given Expr
  Expr* getParentOfSkipPastParImpCastUnOp(Expr* E,bool considerParanAsParent = true) {
    Expr* last_E = nullptr;
    while(1) {
      if(auto ICE = dyn_cast<ImplicitCastExpr>(E)) {
        last_E = E;
        E = cast<Expr>(ICE->getSubExpr());
      }
      else if(auto pExp = dyn_cast<ParenExpr>(E)) {
        if(considerParanAsParent)
          last_E = E;
        E = cast<Expr>(pExp->getSubExpr());
      }
      else if(auto UnOp = dyn_cast<UnaryOperator>(E)) {
        last_E = E;
        E = cast<Expr>(UnOp->getSubExpr());
      }
      else {
        break;
      }
    }
    return last_E;
  }

  // Returns address of Expr node obtained by Skipping 
  // past any parantheses, implicit casts and
  // UnaryOperator expressions from Expr pointed by `E`
  Expr* skipPastParImpCastUnOp(Expr* E) {
    Expr* lastE=nullptr;
    while(1) {
      lastE = E;
      E = E->IgnoreParenImpCasts();
      if(auto UnOp = dyn_cast<UnaryOperator>(E)) {
        E = UnOp->getSubExpr();
      }
      if(E == lastE)
        break;
    }
    return E;
  }

  void DiffRequest::updateCall(FunctionDecl* FD, Sema& SemaRef) {
    CallExpr* call = this->CallContext;
    // Index of "code" parameter:
    auto codeArgIdx = static_cast<int>(call->getNumArgs()) - 1;
    assert(call && "Must be set");
    assert(FD && "Trying to update with null FunctionDecl");

    DeclRefExpr* oldDRE = nullptr;
    // obtain parent of DeclRefExpr of function 
    // to be differentiated
    Expr* oldArgDREParent = call->getArg(0);
    while(1) {
      if(auto skippedToDRE = dyn_cast<DeclRefExpr>(skipPastParImpCastUnOp(oldArgDREParent)))
      {
        if(auto VD = dyn_cast<VarDecl>(skippedToDRE->getDecl())) {
          oldArgDREParent = VD->getInit();
        }
        else {
          break;
        }
      }
      else {
        // Program should not reach here
        llvm_unreachable("Trying to differentiate something unsupported");
        break;
      }
    }
    oldArgDREParent = getParentOfSkipPastParImpCastUnOp(oldArgDREParent,false);
    oldDRE = dyn_cast<DeclRefExpr>(skipPastParImpCastUnOp(oldArgDREParent));

    if(!oldDRE)
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
                                              oldCast->getValueKind());
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

  DeclRefExpr* getArgFunction(CallExpr* E) {
    if (E->getNumArgs() == 0)
      return nullptr;
    Expr* arg = E->getArg(0);
    while(1) {
      arg = skipPastParImpCastUnOp(arg);
      if(auto VD = dyn_cast<VarDecl>(cast<DeclRefExpr>(arg)->getDecl())) {
        arg = VD->getInit();
      }
      else {
        break;
      }
    }
    return dyn_cast<DeclRefExpr>(arg);
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
      DeclRefExpr* DRE = getArgFunction(E);
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
