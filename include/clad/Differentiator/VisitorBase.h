// include/clad/Differentiator/VisitorBase.h
#ifndef CLAD_VISITOR_BASE_H
#define CLAD_VISITOR_BASE_H

#include "clad/Differentiator/CladUtils.h"
#include "clad/Differentiator/DiffPlanner.h" // For DiffRequest
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Sema/Sema.h"

#include <stack>
#include <vector>
#include <unordered_map>

namespace clad {

class DerivativeBuilder; // Forward declaration

/// StmtDiff - a pair of Stmts: the original one and its derivative.
/// If differentiation failed, the second element is nullptr.
class StmtDiff {
  // ... (Full StmtDiff class definition as it exists)
  // For brevity, assume the existing StmtDiff definition is here.
  // Make sure it's complete from the actual VisitorBase.h
public:
  clang::Stmt *S = nullptr, *S_dx = nullptr;
  clang::Expr *valueForRevSweep = nullptr;

  StmtDiff(clang::Stmt* stmt = nullptr, clang::Stmt* stmt_dx = nullptr,
           clang::Expr* valRevSweep = nullptr)
      : S(stmt), S_dx(stmt_dx), valueForRevSweep(valRevSweep) {}

  clang::Expr* getExpr() const { return llvm::dyn_cast_or_null<clang::Expr>(S); }
  clang::Expr* getExpr_dx() const {
    return llvm::dyn_cast_or_null<clang::Expr>(S_dx);
  }
  clang::Stmt* getStmt() const { return S; }
  clang::Stmt* getStmt_dx() const { return S_dx; }
  clang::Expr* getRevSweepAsExpr() const { return valueForRevSweep; }

  void updateStmt(clang::Stmt* stmt) { S = stmt; }
  void updateStmtDx(clang::Stmt* stmt_dx) { S_dx = stmt_dx; }
  void updateRevSweepValue(clang::Expr* val) { valueForRevSweep = val; }

  operator clang::Stmt*() const { return S; }
  bool isValid() const { return S != nullptr; }
};

template <typename T> struct DeclDiff {
  // ... (Full DeclDiff class definition as it exists)
  // For brevity, assume the existing DeclDiff definition is here.
public:
  T *D = nullptr, *D_dx = nullptr;
  DeclDiff(T* d = nullptr, T* d_dx = nullptr) : D(d), D_dx(d_dx) {}
  T* getDecl() const { return D; }
  T* getDecl_dx() const { return D_dx; }
  operator T*() const { return D; }
};


class VisitorBase {
public:
  DerivativeBuilder& m_Builder;
  clang::Sema& m_Sema;
  clang::ASTContext& m_Context;
  const DiffRequest& m_DiffReq;
  clang::Scope* m_Scope = nullptr;
  clang::FunctionDecl* m_Derivative = nullptr;
  clang::Scope* m_DerivativeFnScope = nullptr;

  std::vector<clang::Stmt*> m_BlocksStorage[2]; // 0 for derivative, 1 for primal
  std::vector<std::vector<clang::Stmt*>> m_Blocks; // Stack of blocks

  std::unordered_map<const clang::Decl*, clang::Expr*> m_Variables;
  std::unordered_map<const clang::Decl*, clang::Decl*> m_DeclReplacements;
  clang::Expr* m_ThisExprDerivative = nullptr;

protected:
  // Helper to get unique name
  clang::IdentifierInfo* CreateUniqueIdentifier(llvm::StringRef prefix);

  // Cloners
  clang::Expr* Clone(const clang::Expr* E);
  clang::Stmt* Clone(const clang::Stmt* S);
  template <typename DeclT>
  DeclT* Clone(const DeclT* D);
  clang::ParmVarDecl* CloneParmVarDecl(const clang::ParmVarDecl* PVD,
                                       clang::IdentifierInfo* PVDII,
                                       bool pushOnScopeChains,
                                       bool cloneDefaultArg);
  virtual clang::QualType CloneType(clang::QualType T);

  // Builders
  clang::DeclStmt* BuildDeclStmt(clang::Decl* D);
  clang::DeclStmt* BuildDeclStmt(llvm::ArrayRef<clang::Decl*> Decls);
  clang::DeclRefExpr* BuildDeclRef(clang::ValueDecl* VD,
                                   clang::NestedNameSpecifier* NNS = nullptr,
                                   clang::ExprValueKind VK = clang::VK_LValue);
  clang::Expr* BuildOp(clang::UnaryOperatorKind Op, clang::Expr* E, clang::SourceLocation Loc = clang::SourceLocation());
  clang::Expr* BuildOp(clang::BinaryOperatorKind Op, clang::Expr* LHS, clang::Expr* RHS, clang::SourceLocation Loc = clang::SourceLocation());
  clang::Expr* BuildParens(clang::Expr* E);
  clang::VarDecl* BuildVarDecl(clang::QualType Type, llvm::StringRef Name,
                               clang::Expr* Init = nullptr, bool IsGlobal = false, bool DirectInit = false);
  clang::VarDecl* BuildVarDecl(clang::QualType Type, clang::IdentifierInfo* II,
                               clang::Expr* Init = nullptr, bool IsGlobal = false, bool DirectInit = false);
  clang::VarDecl* BuildGlobalVarDecl(clang::QualType Type, llvm::StringRef Name,
                               clang::Expr* Init = nullptr, bool DirectInit = false);
  clang::VarDecl* BuildGlobalVarDecl(clang::QualType Type, clang::IdentifierInfo* II,
                               clang::Expr* Init = nullptr, bool DirectInit = false);
  clang::CallExpr* BuildCallExprToFunction(clang::FunctionDecl* FD, llvm::ArrayRef<clang::Expr*> Args);
  clang::CallExpr* BuildCallExprToMemFn(clang::Expr* BaseE, llvm::StringRef FnName,
                                        llvm::ArrayRef<clang::Expr*> Args, clang::SourceLocation Loc);
  clang::Expr* BuildOperatorCall(clang::OverloadedOperatorKind Op, llvm::ArrayRef<clang::Expr*> Args);
  clang::CompoundStmt* MakeCompoundStmt(llvm::ArrayRef<clang::Stmt*> Stmts);
  void SetDeclInit(clang::VarDecl* VD, clang::Expr* Init, bool DirectInit = false);

  // Scope Management
  void beginScope(unsigned ScopeFlags);
  void endScope();
  clang::Scope* getCurrentScope() const { return m_Scope; }
  void setCurrentScope(clang::Scope* S) { m_Scope = S; }
  clang::Scope* getEnclosingNamespaceOrTUScope();

  // Diagnostics
  clang::DiagnosticBuilder diag(clang::DiagnosticsEngine::Level DiagLevel,
                                clang::SourceLocation Loc,
                                const char* FormatString);
  template <unsigned N>
  clang::DiagnosticBuilder diag(clang::DiagnosticsEngine::Level DiagLevel,
                                clang::SourceLocation Loc,
                                const char* FormatString,
                                llvm::ArrayRef<llvm::StringRef> Args);

  // Misc Helpers
  bool addToBlock(clang::Stmt* S, std::vector<clang::Stmt*>& Blk);
  clang::Expr* getZeroInit(clang::QualType QT);
  clang::Expr* GetFunctionCall(llvm::StringRef FnName, llvm::StringRef Namespace,
                               llvm::SmallVectorImpl<clang::Expr*>& Args,
                               clang::Expr* CUDAExecConfig = nullptr);
  clang::Stmt* GetCladZeroInit(llvm::ArrayRef<clang::Expr*> Args);
  clang::QualType GetCladTapeOfType(clang::QualType T);
  clang::LookupResult& GetCladTapePush();
  clang::LookupResult& GetCladTapePop();
  clang::LookupResult& GetCladTapeBack();
  clang::Expr* GetCladTapePushDRE();
  void ComputeEffectiveDOperands(StmtDiff& Ldiff, StmtDiff& Rdiff,
                                 clang::Expr*& derivedL, clang::Expr*& derivedR);
  clang::QualType GetPushforwardDerivativeType(const DiffRequest& request, clang::QualType FnRetTy,
                            clang::ArrayRef<clang::QualType> ArgTypes,
                            clang::ArrayRef<clang::QualType> dArgTypes);
  clang::QualType GetValueAndPushforwardPairType(clang::QualType ValueType, clang::QualType PushforwardType);

public:
  VisitorBase(DerivativeBuilder& builder, const DiffRequest& request);
  virtual ~VisitorBase();

  virtual DerivativeAndOverload Derive() = 0;

  // To be defined in VisitorBase.cpp
  clang::QualType GetDerivativeType(const DiffRequest& request); // Removed customParams here to match Turn 19's .cpp

protected:
  // To be defined in VisitorBase.cpp
  clang::QualType GetDerivativeTypeImpl(
      const DiffRequest& request, // Made request const
      llvm::SmallVectorImpl<clang::QualType>& FnParamTypes, // Made non-const ref
      clang::QualType& FnRetTy); // Made non-const ref

  clang::Expr* StoreAndRef(clang::Expr* E, clang::QualType Type,
                           std::vector<clang::Stmt*>& Blk,
                           llvm::StringRef prefix = "_t",
                           bool forceDeclCreation = false);
};

} // namespace clad
#endif // CLAD_VISITOR_BASE_H
