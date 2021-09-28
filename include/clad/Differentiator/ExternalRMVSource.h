#ifndef CLAD_EXTERNAL_RMV_SOURCE_H
#define CLAD_EXTERNAL_RMV_SOURCE_H

#include "llvm/ADT/SmallVector.h"

namespace clang {
  class ValueDecl;
  class CallExpr;
  class FunctionDecl;
  class Expr;
}

namespace clad {

  class ReverseModeVisitor;
  class DiffRequest;
  class StmtDiff;
  class VarDeclDiff;
  // Should we include `DerivativeBuilder.h` instead? `DiffParams` is originally
  // defined in `DerivativeBuilder.h`.
  using DiffParams = llvm::SmallVector<const clang::ValueDecl*, 16>;

  /// An abstract interface that should be implemented by external sources
  /// that provide additional behaviour, in the form of callbacks at crucial
  /// locations, to the reverse mode visitor.
  class ExternalRMVSource {
  public:
    ExternalRMVSource() = default;

    /// Initialise the external source with the ReverseModeVisitor
    virtual void InitialiseRMV(ReverseModeVisitor& RMV) {}

    /// Informs the external source that associated `ReverseModeVisitor`
    /// object is no longer available.
    virtual void ForgetRMV() {};

    /// This is called at the beginning of the `ReverseModeVisitor::Derive`
    /// function.
    virtual void ActOnStartOfDerive() {}

    /// This is called at the end of the `ReverseModeVisitor::Derive`
    /// function.
    virtual void ActOnEndOfDerive() {}

    /// This is called just after differentiation arguments are parsed
    /// in `ReverseModeVisitor::Derive`.
    ///
    ///\param[in] request differentiation request
    ///\param[in] args differentiation args
    virtual void ActAfterParsingDiffArgs(const DiffRequest& request,
                                         DiffParams& args) {}
    virtual void ActBeforeCreatingDerivedFnParamTypes(unsigned &numExtraParam) {}   
    // TODO: Change to ActAfter...                                      
    virtual void ActOnEndOfCreatingDerivedFnParamTypes(llvm::SmallVector<clang::QualType, 16>& paramTypes) {}
    // TODO: Change to ActAfter...
    virtual void ActOnEndOfCreatingDerivedFnParams(llvm::SmallVector<clang::ParmVarDecl*, 4>& params) {}

    /// This is called just before the scopes are created for the derived
    /// function.
    virtual void ActBeforeCreatingDerivedFnScope() {}

    /// This is called just after the scopes for the derived functions have been
    /// created.
    virtual void ActAfterCreatingDerivedFnScope() {}

    /// This is called at the beginning of the derived function body.
    virtual void ActOnStartOfDerivedFnBody(const DiffRequest& request) {}

    /// This is called at the end of the derived function body.
    virtual void ActOnEndOfDerivedFnBody() {}

    virtual void ActAfterProcessingStmtInVisitCompoundStmt() {}
    virtual void ActBeforeFinalizingIfVisitBranchSingleStmt() {}
    virtual void ActAfterProcessingForLoopSingleStmt() {}
    virtual void ActOnEndOfVisitReturnStmt(StmtDiff& ExprDiff) {}
    virtual void ActBeforeFinalizingVisitCallExpr(
        const clang::CallExpr*& CE, clang::Expr*& OverloadedDerivedFn,
        llvm::SmallVector<clang::Expr*, 16>& CallArgs,
        llvm::SmallVector<clang::VarDecl*, 16>& ArgResultDecls) {}
    virtual void ActBeforeFinalizingPostIncDecOp(StmtDiff& diff);
    virtual void
    ActAfterCloningLHSOfAssignOp(clang::Expr*&, clang::Expr*&,
                                 clang::BinaryOperator::Opcode& opCode) {}
    virtual void ActBeforeFinalizingAssignOp(clang::Expr*&, clang::Expr*&);
    virtual void ActOnStartOfDifferentiateSingleStmt(bool&);
    virtual void ActBeforeFinalizingDifferentiateSingleStmt(const ReverseModeVisitor::direction& d) {}
    virtual void ActBeforeFinalizingDifferentiateSingleExpr(const ReverseModeVisitor::direction& d) {}
    virtual void ActAfterDifferentiatingVDInVisitDeclStmt(VarDeclDiff&) {}
  };
} // namespace clad
#endif