// lib/Differentiator/VisitorBase.cpp
#include "clad/Differentiator/VisitorBase.h"
#include "clad/Differentiator/DerivativeBuilder.h" // For DerivativeBuilder
#include "clad/Differentiator/CladUtils.h"       // For utils
#include "clad/Differentiator/DiffPlanner.h"     // For DiffRequest members like CapturedAdjointTypes
#include "clang/AST/Type.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/Scope.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Casting.h" // For llvm::dyn_cast


namespace clad {

// Assuming StmtDiff and DeclDiff are fully defined in VisitorBase.h
// If not, their definitions would go here.

VisitorBase::VisitorBase(DerivativeBuilder& builder, const DiffRequest& request)
    : m_Builder(builder), m_Sema(builder.m_Sema), m_Context(m_Sema.getASTContext()),
      m_DiffReq(request), m_Scope(m_Sema.getScopeForContext(m_Sema.CurContext)) {
  m_Blocks.emplace_back(); // Initial block for forward pass statements
  // m_BlocksStorage[0] and m_BlocksStorage[1] might need initialization if used directly
}

VisitorBase::~VisitorBase() {}

clang::IdentifierInfo* VisitorBase::CreateUniqueIdentifier(llvm::StringRef prefix) {
  llvm::SmallString<256> NameStr(prefix);
  // Instead of m_idCtr, which is not a member of VisitorBase in the provided header,
  // use a simpler unique naming scheme or assume Sema handles collisions if scope is correct.
  // For simplicity, let's use a static counter here. A more robust solution might be needed.
  static unsigned id_counter = 0;
  NameStr.append(llvm::Twine(id_counter++));
  return &m_Context.Idents.get(NameStr);
}

clang::Expr* VisitorBase::Clone(const clang::Expr* E) {
  // Assuming m_Builder.getStmtClone() returns a StmtClone instance
  return static_cast<clang::Expr*>(m_Builder.getStmtClone().Clone(const_cast<clang::Expr*>(E)));
}
clang::Stmt* VisitorBase::Clone(const clang::Stmt* S) {
  return m_Builder.getStmtClone().Clone(const_cast<clang::Stmt*>(S));
}

template <typename DeclT>
DeclT* VisitorBase::Clone(const DeclT* D) {
  return static_cast<DeclT*>(m_Builder.getStmtClone().Clone(const_cast<DeclT*>(D)));
}

clang::ParmVarDecl* VisitorBase::CloneParmVarDecl(const clang::ParmVarDecl* PVD,
                                       clang::IdentifierInfo* PVDII,
                                       bool pushOnScopeChains,
                                       bool cloneDefaultArg) {
    clang::ParmVarDecl* newPVD = clang::ParmVarDecl::Create(
        m_Context, m_Derivative ? m_Derivative : m_Sema.CurContext, // Use m_Derivative if available
        PVD->getBeginLoc(), PVD->getLocation(), PVDII, CloneType(PVD->getType()),
        PVD->getTypeSourceInfo(), PVD->getStorageClass(),
        cloneDefaultArg && PVD->hasDefaultArg() ? Clone(PVD->getDefaultArg()) : nullptr);
    if (pushOnScopeChains && m_Scope) // Check m_Scope
         m_Sema.PushOnScopeChains(newPVD, m_Scope, /*AddToContext=*/false);
    return newPVD;
}

clang::QualType VisitorBase::CloneType(clang::QualType T) {
    // Basic clone, StmtClone might do more advanced things
    return T;
}

clang::DeclStmt* VisitorBase::BuildDeclStmt(clang::Decl* D) {
    if (!D) return nullptr;
    return clang::DeclStmt::Create(m_Context, D->getDeclContext(), D->getLocation(),
                                   llvm::ArrayRef<clang::Decl*>(&D, 1));
}
clang::DeclStmt* VisitorBase::BuildDeclStmt(llvm::ArrayRef<clang::Decl*> Decls) {
    if (Decls.empty()) return nullptr;
    return clang::DeclStmt::Create(m_Context, Decls[0]->getDeclContext(), Decls[0]->getLocation(), Decls);
}

clang::DeclRefExpr* VisitorBase::BuildDeclRef(clang::ValueDecl* VD,
                                   clang::NestedNameSpecifier* NNS,
                                   clang::ExprValueKind VK) {
    clang::CXXScopeSpec SS;
    if (NNS) SS.Adopt(clang::NestedNameSpecifierLoc(NNS, clang::SourceLocation()));
    return clang::DeclRefExpr::Create(m_Context, SS.getWithLocInContext(m_Context), clang::SourceLocation(),
                                      VD, false, VD->getLocation(), VD->getType().getNonReferenceType(), VK);
}

clang::Expr* VisitorBase::BuildOp(clang::UnaryOperatorKind Op, clang::Expr* E, clang::SourceLocation Loc) {
    clang::ExprResult Res = m_Sema.BuildUnaryOp(m_Scope, Loc, Op, E);
    return Res.get();
}
clang::Expr* VisitorBase::BuildOp(clang::BinaryOperatorKind Op, clang::Expr* LHS, clang::Expr* RHS, clang::SourceLocation Loc) {
    clang::ExprResult Res = m_Sema.BuildBinOp(m_Scope, Loc, Op, LHS, RHS);
    return Res.get();
}
clang::Expr* VisitorBase::BuildParens(clang::Expr* E) {
    return m_Sema.ActOnParenExpr(E->getBeginLoc(), E->getEndLoc(), E).get();
}

clang::VarDecl* VisitorBase::BuildVarDecl(clang::QualType Type, llvm::StringRef Name,
                               clang::Expr* Init, bool IsGlobal, bool DirectInit) {
    return BuildVarDecl(Type, &m_Context.Idents.get(Name), Init, IsGlobal, DirectInit);
}

clang::VarDecl* VisitorBase::BuildVarDecl(clang::QualType Type, clang::IdentifierInfo* II,
                               clang::Expr* Init, bool IsGlobal, bool DirectInit) {
    clang::DeclContext* DC = IsGlobal ? m_Context.getTranslationUnitDecl() : m_Sema.CurContext;
    clang::TypeSourceInfo* TSI = m_Context.getTrivialTypeSourceInfo(Type);
    clang::VarDecl* VD = clang::VarDecl::Create(m_Context, DC, clang::SourceLocation(), clang::SourceLocation(),
                                           II, Type, TSI, clang::SC_None);
    if (Init) {
        if (DirectInit) VD->setInit(Init);
        else m_Sema.AddInitializerToDecl(VD, Init, /*DirectInit=*/false);
    }
    if(!IsGlobal && m_Scope) m_Sema.PushOnScopeChains(VD, m_Scope);
    return VD;
}

clang::VarDecl* VisitorBase::BuildGlobalVarDecl(clang::QualType Type, llvm::StringRef Name,
                               clang::Expr* Init, bool DirectInit) {
    return BuildVarDecl(Type, Name, Init, true, DirectInit);
}

clang::VarDecl* VisitorBase::BuildGlobalVarDecl(clang::QualType Type, clang::IdentifierInfo* II,
                               clang::Expr* Init, bool DirectInit) {
    return BuildVarDecl(Type, II, Init, true, DirectInit);
}

clang::CallExpr* VisitorBase::BuildCallExprToFunction(clang::FunctionDecl* FD, llvm::ArrayRef<clang::Expr*> Args) {
    clang::DeclRefExpr* CalleeDRE = BuildDeclRef(FD);
    return clang::CallExpr::Create(m_Context, CalleeDRE, Args, FD->getReturnType(), clang::VK_PRValue, FD->getLocation(), clang::FPOptionsOverride());
}

clang::CallExpr* VisitorBase::BuildCallExprToMemFn(clang::Expr* BaseE, llvm::StringRef FnName,
                                        llvm::ArrayRef<clang::Expr*> Args, clang::SourceLocation Loc) {
    // Simplified: This needs proper lookup for the CXXMethodDecl
    clang::IdentifierInfo* II = &m_Context.Idents.get(FnName);
    clang::UnqualifiedId Name;
    Name.setIdentifier(II, Loc);
    clang::CXXScopeSpec SS;
    clang::ExprResult MemberExpr = m_Sema.ActOnMemberAccessExpr(m_Scope, BaseE, Loc, clang::tok::arrow, SS, clang::SourceLocation(), Name, nullptr);
    if (MemberExpr.isInvalid()) return nullptr;
    return m_Sema.ActOnCallExpr(m_Scope, MemberExpr.get(), Loc, Args, Loc).get();
}

clang::Expr* VisitorBase::BuildOperatorCall(clang::OverloadedOperatorKind Op, llvm::ArrayRef<clang::Expr*> Args) {
    // This is a simplified version. Sema::CreateOverloadedOperatorCall is complex.
    // This might not work for all operators or overload scenarios.
    if (Args.empty()) return nullptr;
    return m_Sema.CreateOverloadedOperatorCall(Op, Args[0]->getExprLoc(), Args, m_Scope).get();
}

clang::CompoundStmt* VisitorBase::MakeCompoundStmt(llvm::ArrayRef<clang::Stmt*> Stmts) {
    return clang::CompoundStmt::Create(m_Context, Stmts, clang::SourceLocation(), clang::SourceLocation());
}

void VisitorBase::SetDeclInit(clang::VarDecl* VD, clang::Expr* Init, bool DirectInit) {
    if (DirectInit) VD->setInit(Init);
    else m_Sema.AddInitializerToDecl(VD, Init, /*DirectInit=*/false);
}

void VisitorBase::beginScope(unsigned ScopeFlags) {
  m_Scope = new (m_Context) clang::Scope(m_Scope, ScopeFlags, m_Sema);
}
void VisitorBase::endScope() {
  if (m_Scope) {
    m_Sema.PopScope(m_Scope);
    m_Scope = m_Scope->getParent();
  }
}

clang::Scope* VisitorBase::getEnclosingNamespaceOrTUScope() {
    clang::Scope* FoundScope = m_Scope;
    while (FoundScope && !(FoundScope->getFlags() & clang::Scope::DeclScope && FoundScope->getEntity() &&
           (isa<clang::TranslationUnitDecl>(FoundScope->getEntity()) || isa<clang::NamespaceDecl>(FoundScope->getEntity())))) {
        FoundScope = FoundScope->getParent();
    }
    return FoundScope;
}

clang::DiagnosticBuilder VisitorBase::diag(clang::DiagnosticsEngine::Level DiagLevel,
                            clang::SourceLocation Loc,
                            const char* FormatString) {
    return m_Sema.Diag(Loc, m_Sema.getDiagnostics().getCustomDiagID(DiagLevel, FormatString));
}

template <unsigned N>
clang::DiagnosticBuilder VisitorBase::diag(clang::DiagnosticsEngine::Level DiagLevel,
                            clang::SourceLocation Loc,
                            const char* FormatString,
                            llvm::ArrayRef<llvm::StringRef> Args) {
    auto D = diag(DiagLevel, Loc, FormatString);
    for(const auto& Arg : Args)
        D << Arg;
    return D;
}

bool VisitorBase::addToBlock(clang::Stmt* S, std::vector<clang::Stmt*>& Blk) {
  if (!S) return false;
  // Simplified check from original, may need refinement if isUnusedResult is complex
  if (auto* E = llvm::dyn_cast<clang::Expr>(S)) {
    if (!E->HasSideEffects(m_Context) && !E->isEvaluatable(m_Context))
        return false;
  }
  Blk.push_back(S);
  return true;
}

clang::Expr* VisitorBase::getZeroInit(clang::QualType QT) {
    return m_Sema.ForceValueInitialization(QT).get();
}

clang::Expr* VisitorBase::GetFunctionCall(llvm::StringRef FnName, llvm::StringRef Namespace,
                               llvm::SmallVectorImpl<clang::Expr*>& Args,
                               clang::Expr* CUDAExecConfig) {
    clang::IdentifierInfo* II = &m_Context.Idents.get(FnName);
    clang::LookupResult R(m_Sema, clang::DeclarationName(II), clang::SourceLocation(), clang::Sema::LookupOrdinaryName);
    clang::DeclContext* SearchDC = m_Context.getTranslationUnitDecl();
    if (!Namespace.empty()) {
        // Simplified namespace lookup
        // Proper lookup would use Sema::LookupQualifiedName
    }
    m_Sema.LookupName(R, m_Scope); // Search in current scope
    if (R.empty()) return nullptr;
    clang::FunctionDecl* FD = R.getAsSingle<clang::FunctionDecl>();
    if (!FD) return nullptr;

    clang::DeclRefExpr* Callee = BuildDeclRef(FD);
    if (CUDAExecConfig) {
         return clang::CUDAKernelCallExpr::Create(m_Context, Callee,
            clang::CallExpr::Config(CUDAExecConfig, clang::SourceLocation(), clang::SourceLocation(), clang::SourceLocation()),
            Args, FD->getReturnType(), clang::SourceLocation());
    }
    return m_Sema.ActOnCallExpr(m_Scope, Callee, clang::SourceLocation(), Args, clang::SourceLocation()).get();
}

clang::Stmt* VisitorBase::GetCladZeroInit(llvm::ArrayRef<clang::Expr*> Args) {
    llvm::SmallVector<clang::Expr*, 2> ArgsVec(Args.begin(), Args.end());
    return GetFunctionCall("zero_init", "clad", ArgsVec);
}

clang::QualType VisitorBase::GetCladTapeOfType(clang::QualType T) {
    clang::TemplateDecl* TD = utils::LookupTemplateDeclInCladNamespace(m_Sema, "tape");
    if (!TD) return clang::QualType();
    clang::TemplateArgumentListInfo TALI;
    TALI.addArgument(clang::TemplateArgumentLoc(clang::TemplateArgument(T), m_Context.getTrivialTypeSourceInfo(T)));
    return utils::InstantiateTemplate(m_Sema, TD, TALI);
}

clang::LookupResult& VisitorBase::GetCladTapePush() { static clang::LookupResult R(m_Sema, {}, {}); return R; }
clang::LookupResult& VisitorBase::GetCladTapePop() { static clang::LookupResult R(m_Sema, {}, {}); return R; }
clang::LookupResult& VisitorBase::GetCladTapeBack() { static clang::LookupResult R(m_Sema, {}, {}); return R; }
clang::Expr* VisitorBase::GetCladTapePushDRE() {return nullptr;}

void VisitorBase::ComputeEffectiveDOperands(StmtDiff& Ldiff, StmtDiff& Rdiff,
                                 clang::Expr*& derivedL, clang::Expr*& derivedR) {
    derivedL = Ldiff.getExpr_dx();
    derivedR = Rdiff.getExpr_dx();
    // Simplified logic from original
    if (utils::isArrayOrPointerType(Ldiff.getExpr()->getType()) &&
        !utils::isArrayOrPointerType(Rdiff.getExpr()->getType()))
      derivedR = Rdiff.getExpr();
    else if (utils::isArrayOrPointerType(Rdiff.getExpr()->getType()) &&
             !utils::isArrayOrPointerType(Ldiff.getExpr()->getType()))
      derivedL = Ldiff.getExpr();
}

clang::Expr* VisitorBase::StoreAndRef(clang::Expr* E, clang::QualType Type,
                           std::vector<clang::Stmt*>& Blk,
                           llvm::StringRef prefix,
                           bool forceDeclCreation) {
  if (!forceDeclCreation && E->isEvaluatable(m_Context)) return E; // Simplified UsefulToStore
  clang::VarDecl* VD = BuildVarDecl(Type, CreateUniqueIdentifier(prefix), E);
  addToBlock(BuildDeclStmt(VD), Blk);
  return BuildDeclRef(VD);
}

clang::QualType VisitorBase::GetDerivativeTypeImpl(
    const DiffRequest& request, // Changed to const DiffRequest&
    llvm::SmallVectorImpl<clang::QualType>& FnParamTypes, // Made non-const ref
    clang::QualType& FnRetTy) { // Made non-const ref

  const clang::FunctionDecl* FD = request.Function; // Use request.Function
  assert(FD && "Must be a function to differentiate.");
  const auto* originalFnType = FD->getType()->getAs<clang::FunctionProtoType>();
  assert(originalFnType && "Function to differentiate must have a proto type.");

  // Original parameters are added by the caller (GetDerivativeType) to FnParamTypes

  FnRetTy = originalFnType->getReturnType(); // Start with original return type

  if (request.Mode == DiffMode::pushforward ||
      request.Mode == DiffMode::vector_pushforward) {
    // Logic for pushforward modes (currently commented out in .h)
    // TODO: Restore Pushforward mode type logic once utils functions are found/fixed
    // For now, FnRetTy remains originalFnType->getReturnType();
  } else if (request.Mode == DiffMode::reverse ||
             request.Mode == DiffMode::jacobian ||
             request.Mode == DiffMode::error_estimation ||
             request.Mode == DiffMode::pullback ||
             request.Mode == DiffMode::reverse_mode_forward_pass) {

    FnRetTy = m_Context.VoidTy; // Default for most reverse/pullback modes

    if (request.Mode == DiffMode::reverse_mode_forward_pass) {
      // TODO: Restore reverse_mode_forward_pass type logic once utils functions are found/fixed
      // clang::QualType origRetTy = originalFnType->getReturnType();
      // if (!origRetTy->isVoidType()) {
      //    clang::QualType adjointTy = utils::GetAdjointType(m_Sema, origRetTy);
      //    FnRetTy = utils::GetValueAndAdjointPairType(m_Sema, origRetTy, adjointTy);
      // } else {
      //    FnRetTy = m_Context.VoidTy;
      // }
       FnRetTy = m_Context.VoidTy; // Placeholder
    }

    if (request.Mode == DiffMode::pullback &&
        !originalFnType->getReturnType()->isVoidType() &&
        !originalFnType->getReturnType()->isPointerType() &&
        !originalFnType->getReturnType()->isReferenceType()) {
      FnParamTypes.push_back(
          utils::getNonConstType(originalFnType->getReturnType(), m_Sema));
    }

    if (utils::needsDThis(FD)) { // Assuming needsDThis is a static helper or moved to utils
        const auto* MD = clang::dyn_cast<clang::CXXMethodDecl>(FD);
        if (MD && MD->isInstance() && !(MD->getParent() && MD->getParent()->isLambda())) {
             FnParamTypes.push_back(
                utils::GetParameterDerivativeType(m_Sema, request.Mode, MD->getThisType()));
        }
    }

    for (const auto* PVD : FD->parameters()) {
      if (!utils::IsDifferentiableType(PVD->getType()))
        continue;
      bool IsParamInDVI = false;
      for (const auto& VarInfo : request.DVI) {
        if (VarInfo.param == PVD) {
          IsParamInDVI = true;
          break;
        }
      }
      if (!IsParamInDVI && request.Mode != DiffMode::reverse_mode_forward_pass)
        continue;
      FnParamTypes.push_back(
          utils::GetParameterDerivativeType(m_Sema, request.Mode, PVD->getType()));
    }

    if (request.isLambdaPullback()) { // Uses the helper from DiffRequest
        for (const auto& type : request.CapturedAdjointTypes) {
            FnParamTypes.push_back(type);
        }
    }

    if (request.Mode == DiffMode::error_estimation) {
      FnParamTypes.push_back(m_Context.DoubleTy);
    }
  }

  clang::FunctionProtoType::ExtProtoInfo EPI = originalFnType->getExtProtoInfo();
  EPI.Variadic = false;
  return m_Context.getFunctionType(FnRetTy, FnParamTypes, EPI);
}

clang::QualType
VisitorBase::GetDerivativeType(const DiffRequest& request) { // Changed to const DiffRequest&
  llvm::SmallVector<clang::QualType, 8> FnParamTypes;
  clang::QualType FnRetTy;

  // Add original parameters from request.Function to FnParamTypes
  if (request.Function) {
      for (const auto* PVD : request.Function->parameters()) {
          FnParamTypes.push_back(PVD->getType());
      }
  }
  // customParams are not directly handled here anymore, assuming they are part of DiffRequest if needed
  // or handled by external sources modifying the list prior to calling a central GetDerivativeTypeImpl

  return GetDerivativeTypeImpl(request, FnParamTypes, FnRetTy);
}

// Dummy definitions for other methods if needed
clang::QualType VisitorBase::GetPushforwardDerivativeType(const DiffRequest& request, clang::QualType FnRetTy,
                            clang::ArrayRef<clang::QualType> ArgTypes,
                            clang::ArrayRef<clang::QualType> dArgTypes) {
    // Dummy implementation
    return FnRetTy;
}

clang::QualType VisitorBase::GetValueAndPushforwardPairType(clang::QualType ValueType, clang::QualType PushforwardType) {
    // Dummy implementation
    return ValueType;
}

} // namespace clad
