#ifndef CLAD_REVERSE_MODE_VERIFIER_H
#define CLAD_REVERSE_MODE_VERIFIER_H
#include "clad/Differentiator/ExternalRMVSource.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>

namespace clang {
  class Sema;
  class FunctionDecl;
  class Expr;
  class ParmVarDecl;
} // namespace clang

namespace clad {
  class ReverseModeVisitor;
  class DiffRequest;

  class ReverseModeVerifier : public ExternalRMVSource {
  public:
    void InitialiseRMV(ReverseModeVisitor& RMV) override;
    void ForgetRMV() override;
    void ActAfterParsingDiffArgs(const DiffRequest& request,
                                 DiffParams& args) override;
    void ActOnStartOfDerivedFnBody(const DiffRequest& request) override;
    void ActOnEndOfDerivedFnBody(const DiffRequest& request, DiffParams& args) override;

  private:
    llvm::ArrayRef<clang::ParmVarDecl*> getNonDiffParams();
    std::string getBaseFilename(std::string filename);
    /// Returns specialization of `clad::VerifyResult` function template
    /// that should be used with `args`.
    clang::FunctionDecl* getCladVerifyResultSpecialization(clang::Sema& SemaRef,
                                                    llvm::ArrayRef<clang::Expr*> args);

  private:
    ReverseModeVisitor* m_RMV = nullptr;
    llvm::SmallVector<clang::FunctionDecl*, 4> m_ForwModeDerivatives;
    llvm::SmallVector<clang::Expr*, 4> m_OriginalArgValues;
  };
} // namespace clad

#endif