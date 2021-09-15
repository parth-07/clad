#include "clad/Differentiator/ReverseModeVerifier.h"

#include "clad/Differentiator/CladUtils.h"
#include "clad/Differentiator/DiffPlanner.h"
// ErrorEstimator.h is required due to some circular dependency design problem
// between ErrorEstimator, DerivativeBuilder and VisitorBase
#include "clad/Differentiator/ErrorEstimator.h"
#include "clad/Differentiator/ReverseModeVisitor.h"

#include "clang/AST/Decl.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/TemplateDeduction.h"

#include <string>

using namespace clang;

namespace clad {
  void ReverseModeVerifier::InitialiseRMV(ReverseModeVisitor& RMV) {
    m_RMV = &RMV;
  }

  void ReverseModeVerifier::ForgetRMV() { m_RMV = nullptr; }

  void ReverseModeVerifier::ActAfterParsingDiffArgs(const DiffRequest& request,
                                                    DiffParams& args) {
    if (request.Mode == DiffMode::reverse &&
        plugin::isReverseModeTestingEnabled(m_RMV->getCladPlugin())) {
      m_ForwModeDerivatives.reserve(args.size());
      auto deriveUsingForwMode = [this, &request, &args](const ValueDecl* arg) {
        // We cannot do testing for the array parameters because we do not know
        // the size of the array parameters at compile time. We need their size
        // at compile time to generate forward mode derivatives for all the
        // valid indexes of the array parameters.
        if (utils::isArrayOrPointerType(arg->getType()))
          return;

        DiffRequest forwModeRequest = request;
        forwModeRequest
            .Args = utils::CreateStringLiteral(m_RMV->getASTContext(),
                                               arg->getName());
        forwModeRequest.Mode = DiffMode::forward;
        forwModeRequest.CallUpdateRequired = false;
        FunctionDecl*
            firstDerivative = plugin::ProcessDiffRequest(m_RMV->getCladPlugin(),
                                                         forwModeRequest);
        m_ForwModeDerivatives.push_back(firstDerivative);
      };
      std::for_each(args.begin(), args.end(), deriveUsingForwMode);
    }
  }

  void
  ReverseModeVerifier::ActOnStartOfDerivedFnBody(const DiffRequest& request) {
    if (request.Mode != DiffMode::reverse ||
        !plugin::isReverseModeTestingEnabled(m_RMV->getCladPlugin()))
      return;
    auto nonDiffParams = getNonDiffParams();
    m_OriginalArgValues.reserve(nonDiffParams.size());
    for (ParmVarDecl* param : nonDiffParams) {
      auto paramCopy = m_RMV->BuildVarDecl(param->getType()
                                               .getNonReferenceType(),
                                           "_p_" + param->getNameAsString(),
                                           m_RMV->BuildDeclRef(param));
      m_OriginalArgValues.push_back(m_RMV->BuildDeclRef(paramCopy));
      m_RMV->addToBlock(m_RMV->BuildDeclStmt(paramCopy), m_RMV->getGlobals());
    }
  }

  llvm::ArrayRef<ParmVarDecl*> ReverseModeVerifier::getNonDiffParams() {
    auto params = m_RMV->m_Derivative->parameters();
    auto arrRefType = m_RMV->GetCladArrayRefOfType(
        this->m_RMV->m_Function->getReturnType());
    auto DiffParamsStart = std::find_if(params.begin(), params.end(),
                                        [arrRefType](ParmVarDecl* VD) {
                                          return VD->getType()
                                                     .getCanonicalType() ==
                                                 arrRefType.getCanonicalType();
                                        });
    return llvm::ArrayRef<ParmVarDecl*>(params.begin(), DiffParamsStart);
  }

  void ReverseModeVerifier::ActOnEndOfDerivedFnBody(const DiffRequest& request,
                                                    DiffParams& args) {
    if (request.Mode != DiffMode::reverse ||
        !plugin::isReverseModeTestingEnabled(m_RMV->getCladPlugin()))
      return;
    Sema& semaRef = m_RMV->getSema();
    ASTContext& C = m_RMV->getASTContext();

    size_t forwDerivedFnCounter = 0;
    std::string completeFileName = semaRef.SourceMgr
                                       .getFilename(
                                           m_RMV->m_Function->getBeginLoc())
                                       .str();
    std::string baseFileName = getBaseFilename(completeFileName);
    std::string qualifiedGradientName = m_RMV->m_Derivative
                                            ->getQualifiedNameAsString();
    auto nonDiffParams = getNonDiffParams();
    for (const auto& arg : args) {
      // We cannot do testing for the array parameters because we do not know
      // the size of the array parameters at compile time. We need their size
      // at compile time to generate forward mode derivatives for all the
      // valid indexes of the array parameters.
      if (utils::isArrayOrPointerType(arg->getType()))
        continue;
      auto& forwDerivedFn = m_ForwModeDerivatives[forwDerivedFnCounter];
      ++forwDerivedFnCounter;
      Expr* forwDiffResult = m_RMV
                                 ->BuildCallExprToFunction(forwDerivedFn,
                                                           m_OriginalArgValues);
      Expr* revDiffResult = m_RMV->m_Variables[arg];
      std::string
          assertMessage = "Inconsistent differentiation result with respect "
                          "to the parameter '" +
                          arg->getNameAsString() +
                          "' in forward and reverse differentiation mode";
      auto fileNameLiteral = utils::CreateStringLiteral(C, baseFileName);
      auto fnNameLiteral = utils::CreateStringLiteral(C, qualifiedGradientName);
      auto assertMessageLiteral = utils::CreateStringLiteral(C, assertMessage);

      std::vector<Expr*> verifyResultArgs = {revDiffResult, forwDiffResult,
                                             fileNameLiteral, fnNameLiteral,
                                             assertMessageLiteral};
      FunctionDecl*
          verifyResultFD = getCladVerifyResultSpecialization(m_RMV->getSema(),
                                                             verifyResultArgs);
      CXXScopeSpec CSS;
      CSS.Extend(C, m_RMV->GetCladNamespace(), noLoc, noLoc);
      auto verifyResultDRE = m_RMV->getSema()
                                 .BuildDeclarationNameExpr(CSS,
                                                           verifyResultFD
                                                               ->getNameInfo(),
                                                           verifyResultFD)
                                 .get();
      Expr* verifyResultCallExpr = m_RMV->getSema()
                                       .ActOnCallExpr(m_RMV->getCurrentScope(),
                                                      verifyResultDRE,
                                                      /*LParenLoc=*/noLoc,
                                                      verifyResultArgs,
                                                      /*RParenLoc=*/noLoc)
                                       .get();
      m_RMV->addToCurrentBlock(verifyResultCallExpr, m_RMV->forward);
    }
  }

  std::string ReverseModeVerifier::getBaseFilename(std::string filename) {
    std::string baseFilename = filename;
    auto lastForwardSlash = filename.rfind('/');
    if (lastForwardSlash != std::string::npos)
      baseFilename = filename.substr(lastForwardSlash + 1);
    return baseFilename;
  }

  FunctionDecl* ReverseModeVerifier::getCladVerifyResultSpecialization(
      Sema& SemaRef, llvm::ArrayRef<Expr*> args) {
    FunctionDecl* specialization = nullptr;

    ASTContext& C = SemaRef.getASTContext();
    // looking up `clad` namespace declaration.
    IdentifierInfo& cladIdentifier = C.Idents.get("clad");
    LookupResult cladR(SemaRef, DeclarationName(&cladIdentifier), noLoc,
                       Sema::LookupNamespaceName,
                       clad_compat::Sema_ForVisibleRedeclaration);
    SemaRef.LookupQualifiedName(cladR, C.getTranslationUnitDecl());
    NamespaceDecl* cladND = cast<NamespaceDecl>(cladR.getFoundDecl());

    // looking up `clad::VerifyResult` template declaration.
    IdentifierInfo& VerifyResultIdentifier = C.Idents.get("VerifyResult");
    CXXScopeSpec SS;
    SS.Extend(C, cladND, noLoc, noLoc);
    LookupResult R(SemaRef, DeclarationName(&VerifyResultIdentifier), noLoc,
                   Sema::LookupNameKind::LookupOrdinaryName);
    // R.setTemplateNameLookup(true);
    SemaRef.LookupQualifiedName(R, cladND, SS);
    for (auto decl : R) {
      if (auto templ = dyn_cast<FunctionTemplateDecl>(decl)) {
        sema::TemplateDeductionInfo TDI(noLoc);
        SemaRef.DeduceTemplateArguments(templ, nullptr, args, specialization,
                                        TDI, false,
                                        [](llvm::ArrayRef<QualType>) {
                                          return false;
                                        });
        if (specialization)
          return specialization;
      }
    }
    return nullptr;
  }
} // namespace clad