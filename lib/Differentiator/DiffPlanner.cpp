#include "clad/Differentiator/DiffPlanner.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/TemplateDeduction.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/SaveAndRestore.h"

#include "clad/Differentiator/Compatibility.h"

using namespace clang;

namespace clad {
  static SourceLocation noLoc;

  // 
  class DeclRefVisitor 
    : public ConstStmtVisitor<DeclRefVisitor,const Expr*> {
  private:
    using StmtClassSet = llvm::SmallSet<Stmt::StmtClass,16U>;
  public:
    DeclRefVisitor(const llvm::ArrayRef<Stmt::StmtClass>& pStmtTypesToSkip,
                  const llvm::ArrayRef<Stmt::StmtClass>& pStmtTypesToConsiderForAnc) {
      stmtTypesToSkip.insert(pStmtTypesToSkip.begin(),pStmtTypesToSkip.end());
      stmtTypesToConsiderForAnc.insert(pStmtTypesToConsiderForAnc.begin(),
                                      pStmtTypesToConsiderForAnc.end()); 
    }
    
    // Returns nearest DeclRefExpr descendant of expression 
    // pointed by `E` by skipping past 0 or more statements.
    // Only stmts specified to be skipped can be skipped.
    // Returns nullptr, if DeclRefExpr cannot be reached by 
    // skipping past the specified stmts.
    const DeclRefExpr* skipToDeclRef(const Expr* E) {
      relevantAncestor = nullptr;
      return dyn_cast_or_null<const DeclRefExpr>(Visit(E));
    }

    // Returns nearest function declaration DeclRefExpr descendant of
    // expression pointed by `E` by skipping past 0 or more statments
    // and declarations.
    // Only stmts specified to be skipped can be skipped.
    // Returns nullptr, if function declaration DeclRefExpr cannot be 
    // reached by skipping past the declarations and specified stmts.
    const DeclRefExpr* skipToFunctionDeclRef(const Expr* E) {
      relevantAncestor = nullptr;
      const DeclRefExpr* DRE = nullptr;
      
      while (DRE = dyn_cast_or_null<const DeclRefExpr>(skipToDeclRef(E))) {
        if (auto VD = dyn_cast<const VarDecl>(DRE->getDecl())) {
          E = VD->getInit();
        } else {
          break;
        }
      }

      return DRE;
    }

    // Returns relevant ancestor of nearest DeclRefExpr descendant of
    // expression pointed by `E`.
    // Relevant ancestor is the nearest stmt ancestor allowed to 
    // be considered to be ancestor 
    const Expr* skipToDeclRefrelevantAncestor(const Expr* E) {
      skipToDeclRef(E);
      return relevantAncestor;
    }

    // Returns relevant ancestor of nearest function declaration
    // DeclRefExpr descendant of expression pointed by `E`.
    // Relevant ancestor is the nearest stmt ancestor allowed to 
    // be considered to be ancestor 
    const Expr* skipToFunctionDeclRefrelevantAncestor(const Expr* E) {
      skipToFunctionDeclRef(E);
      return relevantAncestor;
    }

    const Expr* getrelevantAncestor() const {
      return relevantAncestor;
    }

    const Expr* VisitImplicitCastExpr(const ImplicitCastExpr* ICE) {
      if (!isSkippable(ICE)) {
        return nullptr;
      }
      updaterelevantAncestor(ICE);
      return Visit(ICE->getSubExpr());
    }
    const Expr* VisitParenExpr(const ParenExpr* PE) {
      if (!isSkippable(PE)) {
        return nullptr;
      }
      updaterelevantAncestor(PE);
      return Visit(PE->getSubExpr());
    }
    const Expr* VisitUnaryOperator(const UnaryOperator* UnOp) {
      if(!isSkippable(UnOp)) {
        return nullptr;
      }
      updaterelevantAncestor(UnOp);
      return Visit(UnOp->getSubExpr());
    }
    const Expr* VisitDeclRefExpr(const DeclRefExpr* DRE) {
      return DRE;
    }
  private:
    
    // if stmtTypesToSkip is empty, then all stmt types other
    // than DeclRefExpr will be skipped
    StmtClassSet stmtTypesToSkip;
    
    // if stmtTypesToConsiderForAnc is empty, then all stmt types
    // will be considered for ancestor 
    StmtClassSet stmtTypesToConsiderForAnc;
    
    const Expr* relevantAncestor = nullptr;
    void updaterelevantAncestor(const Expr* E) {
      if ( stmtTypesToConsiderForAnc.empty() || 
          stmtTypesToConsiderForAnc.count(E->getStmtClass())) {
        relevantAncestor = E;
      }
    }

    bool isSkippable(const Expr* E) {
      return stmtTypesToSkip.empty() || stmtTypesToSkip.count(E->getStmtClass());
    }
  };

  class DummyVisitor : public ConstStmtVisitor<DummyVisitor,const Expr*> {
    private:
    const Expr* parent = nullptr;
    public:
    const Expr* getDRENonParanParent(const Expr* E) {
      return Visit(E);
    }
    const Expr* VisitImplicitCastExpr(const ImplicitCastExpr* ICE) {
      llvm::errs()<<"ICE: "<<ICE->getStmtClassName()<<"\n";
      parent = ICE;
      if (auto dre = dyn_cast<DeclRefExpr>(ICE->getSubExpr())){
        return parent;
      }
      return Visit(ICE->getSubExpr());
    }
    const Expr* VisitParenExpr(const ParenExpr* PE) {
      if (auto dre = dyn_cast<DeclRefExpr>(PE->getSubExpr())) {
        return parent;
      }
      return PE->getSubExpr();
    }
    const Expr* VisitUnaryOperator(const UnaryOperator* UnOp) {
      parent = UnOp;
      if (auto dre = dyn_cast<DeclRefExpr>(UnOp->getSubExpr())) {
        return parent;
      }
      return UnOp->getSubExpr();
    }
  } dummyVisitor;


  // Returns address of parent of Expr node obtained by skipping 
  // past any parantheses, implicit casts and UnaryOperators 
  // from Expr pointed by `E`.
  // If considerParanAsParent is false,then returns last non-parantheses
  // ancestor of Expr node obtained by skipping.
  // Returns nullptr, if there is no skipping of nodes from given Expr
  Expr* getParentOfSkipPastParImpCastUnOp(Expr* E,
                                          bool considerParanAsParent = true) {
    Expr* last_E = nullptr;
    while (1) {
      if (auto ICE = dyn_cast<ImplicitCastExpr>(E)) {
        last_E = E;
        E = cast<Expr>(ICE->getSubExpr());
      } else if (auto pExp = dyn_cast<ParenExpr>(E)) {
        if (considerParanAsParent)
          last_E = E;
        E = cast<Expr>(pExp->getSubExpr());
      } else if (auto UnOp = dyn_cast<UnaryOperator>(E)) {
        last_E = E;
        E = cast<Expr>(UnOp->getSubExpr());
      } else {
        break;
      }
    }
    return last_E;
  }

  // Returns address of Expr node obtained by Skipping 
  // past any parantheses, implicit casts and
  // UnaryOperator expressions from Expr pointed by `E`
  const Expr* skipPastParImpCastUnOp(const Expr* E) {
    const Expr* lastE=nullptr;
    while (1) {
      lastE = E;
      E = E->IgnoreParenImpCasts();
      if (auto UnOp = dyn_cast<const UnaryOperator>(E)) {
        E = UnOp->getSubExpr();
      }
      if (E == lastE)
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

    const DeclRefExpr* oldDRE = nullptr;
    // obtain parent of DeclRefExpr of function to be differentiated
    const Expr* oldArgDREParent = call->getArg(0);
    // while (1) {
    //   if (auto skippedToDRE 
    //       = dyn_cast<const DeclRefExpr>(skipPastParImpCastUnOp(oldArgDREParent))) {
    //     if (auto VD = dyn_cast<const VarDecl>(skippedToDRE->getDecl())) {
    //       oldArgDREParent = VD->getInit();
    //     } else {
    //       break;
    //     }
    //   }
    // }
    DeclRefVisitor dreVisitor({
      Stmt::StmtClass::ImplicitCastExprClass,
      Stmt::StmtClass::UnaryOperatorClass,
      Stmt::StmtClass::ParenExprClass
    },  {
      Stmt::StmtClass::ImplicitCastExprClass,
      Stmt::StmtClass::UnaryOperatorClass
    });
    oldArgDREParent = dreVisitor.skipToFunctionDeclRefrelevantAncestor(call->getArg(0));
    oldDRE = dreVisitor.skipToFunctionDeclRef(oldArgDREParent);

    // auto tempDre = dummyVisitor.getDRENonParanParent(call->getArg(0));
    // if(tempDre == oldArgDREParent) {
    //   llvm::errs()<<"Tada, both are same"<<"\n";
    // }
    // else {
    //   llvm::errs()<<"Alas\n";
    // }

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
    while (1) {
      arg = const_cast<Expr*>(skipPastParImpCastUnOp(arg));
      if (auto VD = dyn_cast<VarDecl>(cast<DeclRefExpr>(arg)->getDecl())) {
        arg = VD->getInit();
      } else {
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
      DeclRefExpr* DRE = nullptr;
      DeclRefVisitor dreVisitor({},{});
      if(E->getNumArgs() > 0)
        DRE = const_cast<DeclRefExpr*>(dreVisitor.skipToFunctionDeclRef(E->getArg(0)));
      
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
