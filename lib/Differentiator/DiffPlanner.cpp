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

  // returns pointer to parent of first DeclRefExpr descendant 
  // of Expr pointed by E in AST.
  // returns nullptr, if node E is itself of type DeclRefExpr
  // returns nullptr, if node of type other than ImplicitCastExpr
  // or UnaryOperator comes before DeclRefExpr
  Expr* getParentOfFirstDreDescendant(Expr* E) {
    Expr* last_E = nullptr;
    while(1) {
      if(auto ICE = dyn_cast<ImplicitCastExpr>(E)) {
        last_E = E;
        E = cast<Expr>(ICE->getSubExpr());
      }
      else if(auto UnOp = dyn_cast<UnaryOperator>(E)) {
        last_E = E;
        E = cast<Expr>(UnOp->getSubExpr());
      }
      else {
        break;
      }
    }
    if(isa<DeclRefExpr>(E))
      return last_E;
    else 
      return nullptr;
  }

  // Returns pointer to first DeclRefExpr descendant of Expr 
  // pointed by E in AST.
  // returns E if Expr pointed by E is itself of type DeclRefExpr
  // return nullptr, if node of type other than ImplicitCastExpr
  // or UnaryOperator comes before DeclRefExpr
  DeclRefExpr* getFirstDreDescendant(Expr* E) {
    if(isa<DeclRefExpr>(E))
      return dyn_cast<DeclRefExpr>(E);
    Expr* lastNonDreExp = getParentOfFirstDreDescendant(E);
    
    if(auto ICE = dyn_cast<ImplicitCastExpr>(lastNonDreExp)) {
      lastNonDreExp = cast<Expr>(ICE->getSubExpr());
    }
    else if(auto UnOp = dyn_cast<UnaryOperator>(lastNonDreExp)) {
      lastNonDreExp = cast<Expr>(UnOp->getSubExpr());
    }
    return dyn_cast<DeclRefExpr>(lastNonDreExp);
  }

  void DiffRequest::updateCall(FunctionDecl* FD, Sema& SemaRef) {
    CallExpr* call = this->CallContext;
    // Index of "code" parameter:
    auto codeArgIdx = static_cast<int>(call->getNumArgs()) - 1;
    assert(call && "Must be set");
    assert(FD && "Trying to update with null FunctionDecl");

    DeclRefExpr* oldDRE = nullptr;
    oldDRE = getArgFunction(call);
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
    Expr* argFnParent = call->getArg(0);
    while(1) {
      if(auto VD = dyn_cast<VarDecl>(getFirstDreDescendant(argFnParent)->getDecl())) {
        argFnParent = VD->getInit();
      }
      else {
        break;
      }
    }
    argFnParent=getParentOfFirstDreDescendant(argFnParent);
    if (auto oldCast = dyn_cast<ImplicitCastExpr>(argFnParent)) {
      // Cast function to function pointer.
      auto newCast = ImplicitCastExpr::Create(C,
                                              C.getPointerType(FD->getType()),
                                              oldCast->getCastKind(),
                                              DRE,
                                              nullptr,
                                              oldCast->getValueKind());
      call->setArg(0, newCast);
    }
    else if (auto oldUnOp = dyn_cast<UnaryOperator>(argFnParent)) {
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
    // llvm::errs()<<"FD info"<<"\n";
    // auto FDQualType = cast<CXXMethodDecl>(FD)->getThisObjectType();
    // llvm::errs()<<"type = "<<FDQualType.getAsString()<<"\n";
    // FDQualType.dump();
    // call->setType(FD->getType());
    // llvm::errs()<<"directCalle info\n";
    // auto directCallee = call->getDirectCallee();
    // auto returnType = directCallee->getReturnType();
    // llvm::errs()<<"name = "<<directCallee->getName()<<"\n";
    // llvm::errs()<<"type = "<<returnType.getAsString()<<"\n";
    // llvm::errs()<<"callExpr type = "<<call->getType().getAsString()<<"\n";
    // llvm::errs()<<"callReturnType = "<<call->getCallReturnType(C).getAsString()<<"\n";

    // llvm::errs()<<"call->getType() info\n";
    // auto callQualType = call->getType();
    // llvm::errs()<<"type = "<< callQualType.getAsString()<<"\n";
    // callQualType.dump();

    auto memPtrType = SemaRef.BuildMemberPointerType(FD->getType(),dyn_cast<CXXMethodDecl>(FD)->getThisObjectType(),
                                                     noLoc,DeclarationName());
      call->setType(memPtrType);
      llvm::errs()<<"memPtrType name = "<<memPtrType.getAsString()<<"\n";
    
    

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

    llvm::errs()<<"call return type info\n";
    auto ReturnType = call->getCallReturnType(C);
    llvm::errs()<<ReturnType.getAsString()<<"\n";
    if(call->getType() != ReturnType) {
      call->setType(ReturnType);
      llvm::errs()<<"Here we are = "<<call->getType().getAsString()<<"\n";
      auto start = C.getParents(*call).begin();
      auto e = C.getParents(*call).end();
      auto it = start;
      size_t counter = 0;
      size_t sz = C.getParents(*call).size();
      size_t temp_sz;
      bool b_flag=0;
      while(1) {
        llvm::errs()<<"sz = "<<sz<<"\n";
        while(1) {
          if(auto node = start->get<Expr>()) {
            llvm::errs()<<"node name = "<<"some expr\n";
            const_cast<Expr*>(node)->setType(ReturnType);
            it = C.getParents(*node).begin();
            temp_sz = C.getParents(*node).size();
            // if(auto cnode = dyn_cast<CXXConstructExpr>(node)) {
            //   auto cladFunctionSpecializedClass = ReturnType->getAsCXXRecordDecl();
              
            //   auto cexpr = CXXConstructExpr::Create(
            //     C,
            //     ReturnType,
            //     noLoc,
            //     *(cladFunctionSpecializedClass->ctor_begin()),
            //     cnode->isElidable(),
            //     ArrayRef<Expr*>(const_cast<cnode->getArgs()),
            //     cnode->hadMultipleCandidates(),
            //     cnode->isListInitialization(),
            //     cnode->isStdInitListInitialization(),
            //     cnode->requiresZeroInitialization(),
            //     cnode->getConstructionKind(),
            //     cnode->getSourceRange()
            //   )
            // }
          }
          else if(auto node = start->get<ValueDecl>()) {
            llvm::errs()<<"node name = "<<node->getName()<<"\n";
            const_cast<ValueDecl*>(node)->setType(ReturnType);
            it = C.getParents(*node).begin();
            temp_sz = C.getParents(*node).size();
          }
          else if(auto node = start->get<Stmt>()){
            llvm::errs()<<"in stmt"<<"\n";
            it = C.getParents(*node).begin();
            temp_sz = C.getParents(*node).size();
            b_flag=1;
            break;
          }
          else {
            llvm::errs()<<"Not Breaking here\n";
            // break;
          }

          if(auto node = start->get<VarDecl>()) {
            SemaRef.AddInitializerToDecl(const_cast<VarDecl*>(node),call,true);
            // const_cast<VarDecl*>(node)->setInit(call);
            // const_cast<VarDecl*>(node)->setType(ReturnType);
            b_flag=1;
            break;
          }
          start = start + 1;
          ++counter;
          llvm::errs()<<"Reached here, incrementing start\n";
          if(counter == sz) {
            break;
          }
        }
        if(b_flag)
          break;
        start = it;
        sz = temp_sz;
        counter=0;
      }
    }

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
      arg = getFirstDreDescendant(arg);
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
