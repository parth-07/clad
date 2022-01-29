/// This file contains functions for easily querying, modifying and creating
/// AST nodes.

#ifndef CLAD_AST_HELPERS_H
#define CLAD_AST_HELPERS_H

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/Type.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Sema.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "clad/Differentiator/Compatibility.h"

#include <functional>
#include <initializer_list>
namespace clad {
  namespace ast_helper {
    class ScopeHandler {
      clang::Sema& m_Sema;

    public:
      clang::Scope*& m_CurScope;

      ScopeHandler(clang::Sema& semaRef, clang::Scope*& curScope)
          : m_CurScope(curScope), m_Sema(semaRef) {}
      void BeginScope(unsigned scopeFlags);
      void EndScope();
      clang::Scope* GetCurrentScope() { return m_CurScope; }
    };
  } // namespace ast_helper
  class ASTHelper {
    clang::Sema& m_Sema;
    clang::ASTContext& m_Context;

  public:
    struct CustomScope {
      static const unsigned
          FunctionBeginScope = clang::Scope::FunctionPrototypeScope |
                               clang::Scope::FunctionDeclarationScope |
                               clang::Scope::DeclScope;
      static const unsigned FunctionBodyScope = clang::Scope::FnScope |
                                                clang::Scope::DeclScope;
    };
    ASTHelper(clang::Sema& sema);

    clang::CXXRecordDecl* FindCXXRecordDecl(clang::DeclarationName recordName);
    static clang::CXXRecordDecl*
    FindCXXRecordDecl(clang::Sema& semaRef, clang::DeclarationName recordName);

    clang::QualType GetFundamentalType(llvm::StringRef typeName) const;
    static clang::QualType GetFundamentalType(clang::Sema& semaRef,
                                              llvm::StringRef typeName);

    bool IsFundamentalType(llvm::StringRef name) const;
    static bool IsFundamentalType(clang::Sema& semaRef, llvm::StringRef name);

    clang::QualType ComputeQTypeFromTypeName(llvm::StringRef typeName) const;
    static clang::QualType ComputeQTypeFromTypeName(clang::Sema& semaRef,
                                                    llvm::StringRef typeName);

    clang::ValueDecl* FindRecordDeclMember(clang::CXXRecordDecl* RD,
                                           llvm::StringRef memberName);

    static clang::ValueDecl* FindRecordDeclMember(clang::Sema& semaRef,
                                                  clang::CXXRecordDecl* RD,
                                                  llvm::StringRef memberName);

    clang::MemberExpr* BuildMemberExpr(clang::Expr* base,
                                       clang::ValueDecl* member);

    static clang::MemberExpr* BuildMemberExpr(clang::Sema& semaRef,
                                              clang::Expr* base,
                                              clang::ValueDecl* member);

    clang::CXXNewExpr* BuildNewExprFor(clang::QualType qType,
                                       clang::Expr* initializer,
                                       clang::SourceLocation B);

    static clang::CXXNewExpr* BuildNewExprFor(clang::Sema& semaRef,
                                              clang::QualType qType,
                                              clang::Expr* initializer,
                                              clang::SourceLocation B);

    clang::CXXConstructorDecl* FindCopyConstructor(clang::CXXRecordDecl* RD);

    static clang::CXXConstructorDecl*
    FindCopyConstructor(clang::Sema& semaRef, clang::CXXRecordDecl* RD);

    clang::FunctionDecl* FindUniqueFnDecl(clang::DeclContext* DC,
                                          clang::DeclarationName fnName);

    static clang::FunctionDecl* FindUniqueFnDecl(clang::Sema& semaRef,
                                                 clang::DeclContext* DC,
                                                 clang::DeclarationName fnName);

    clang::DeclarationName BuildDeclName(llvm::StringRef name);
    static clang::DeclarationName BuildDeclName(clang::Sema& semaRef,
                                                llvm::StringRef name);

    clang::DeclarationNameInfo BuildDeclNameInfo(llvm::StringRef name);
    static clang::DeclarationNameInfo BuildDeclNameInfo(clang::Sema& semaRef,
                                                        llvm::StringRef name);

    clang::Expr* IgnoreParenImpCastsUnaryOp(clang::Expr* E);

    static clang::Expr* IgnoreParenImpCastsUnaryOp(clang::Sema& semaRef,
                                                   clang::Expr* E);

    clang::Expr* BuildCXXCopyConstructExpr(clang::QualType qType,
                                           clang::Expr* E);
    // NOTE: This currently only creates default construct expressions for class
    // types
    static clang::Expr* BuildCXXCopyConstructExpr(clang::Sema& semaRef,
                                                  clang::QualType qType,
                                                  clang::Expr* E);

    clang::QualType FindCorrespondingType(llvm::StringRef name);
    static clang::QualType FindCorrespondingType(clang::Sema& semaRef,
                                                 llvm::StringRef name);

    clang::NamespaceDecl* FindCladNamespace();
    static clang::NamespaceDecl* FindCladNamespace(clang::Sema& semaRef);

    clang::SourceLocation GetValidSL();
    static clang::SourceLocation GetValidSL(clang::Sema& semaRef);

    clang::SourceRange GetValidSR();
    static clang::SourceRange GetValidSR(clang::Sema& semaRef);

    clang::VarDecl* BuildVarDecl(clang::DeclContext* DC,
                                 clang::IdentifierInfo* II,
                                 clang::QualType qType);
    static clang::VarDecl* BuildVarDecl(clang::Sema& semaRef,
                                        clang::DeclContext* DC,
                                        clang::IdentifierInfo* II,
                                        clang::QualType qType);

    clang::ParmVarDecl* BuildParmVarDecl(clang::DeclContext* DC,
                                         clang::IdentifierInfo* II,
                                         clang::QualType qType);
    static clang::ParmVarDecl* BuildParmVarDecl(clang::Sema& semaRef,
                                                clang::DeclContext* DC,
                                                clang::IdentifierInfo* II,
                                                clang::QualType qType);

    clang::DeclRefExpr*
    BuildDeclRefExpr(clang::ValueDecl* VD,
                     clang::QualType qType = clang::QualType());
    static clang::DeclRefExpr*
    BuildDeclRefExpr(clang::Sema& semaRef, clang::ValueDecl* VD,
                     clang::QualType qType = clang::QualType());
    clang::DeclStmt* BuildDeclStmt(clang::Decl* D);
    static clang::DeclStmt* BuildDeclStmt(clang::Sema& semaRef, clang::Decl* D);

    clang::FieldDecl*
    BuildFieldDecl(clang::DeclContext* DC, clang::IdentifierInfo* II,
                   clang::QualType qType, clang::Expr* init = nullptr,
                   clang::AccessSpecifier AS = clang::AccessSpecifier::AS_none,
                   bool addToDecl = false);
    static clang::FieldDecl*
    BuildFieldDecl(clang::Sema& sema, clang::DeclContext* DC,
                   clang::IdentifierInfo* II, clang::QualType qType,
                   clang::Expr* init = nullptr,
                   clang::AccessSpecifier AS = clang::AccessSpecifier::AS_none,
                   bool addToDecl = false);

    void RegisterFn(clang::DeclContext* DC, clang::FunctionDecl* FD);
    static void RegisterFn(clang::Sema& semaRef, clang::DeclContext* DC,
                           clang::FunctionDecl* FD);

    clang::CompoundStmt* BuildCompoundStmt(llvm::ArrayRef<clang::Stmt*> block);
    static clang::CompoundStmt*
    BuildCompoundStmt(clang::Sema& semaRef, llvm::ArrayRef<clang::Stmt*> block);

    clang::FunctionDecl* BuildFnDecl(clang::DeclContext* DC,
                                     clang::DeclarationNameInfo nameInfo,
                                     clang::QualType fnQType);
    static clang::FunctionDecl* BuildFnDecl(clang::Sema& semaRef,
                                            clang::DeclContext* DC,
                                            clang::DeclarationNameInfo nameInfo,
                                            clang::QualType fnQType);

    clang::CXXMethodDecl* BuildMemFnDecl(clang::CXXRecordDecl* RD,
                                         clang::DeclarationNameInfo nameInfo,
                                         clang::QualType qType);
    static clang::CXXMethodDecl*
    BuildMemFnDecl(clang::Sema& semaRef, clang::CXXRecordDecl* RD,
                   clang::DeclarationNameInfo nameInfo, clang::QualType qType);

    clang::Expr* BuildCallToFn(clang::Scope* S, clang::FunctionDecl* FD,
                               llvm::MutableArrayRef<clang::Expr*> args);
    clang::Expr* BuildCallToFn(clang::Sema& semaRef, clang::Scope* S,
                               clang::FunctionDecl* FD,
                               llvm::MutableArrayRef<clang::Expr*> args);

    clang::Expr* BuildCallToMemFn(clang::Scope* S, clang::Expr* base,
                                  clang::CXXMethodDecl* memFn,
                                  llvm::MutableArrayRef<clang::Expr*> args);
    static clang::Expr*
    BuildCallToMemFn(clang::Sema& semaRef, clang::Scope* S, clang::Expr* base,
                     clang::CXXMethodDecl* memFn,
                     llvm::MutableArrayRef<clang::Expr*> args);

    clang::ReturnStmt* BuildReturnStmt(clang::Expr* retValExpr,
                                       clang::Scope* curScope);
    static clang::ReturnStmt* BuildReturnStmt(clang::Sema& semaRef,
                                              clang::Expr* retValExpr,
                                              clang::Scope* curScope);

    clang::Expr* BuildOp(clang::BinaryOperatorKind opCode, clang::Expr* L,
                         clang::Expr* R, clang::Scope* S = nullptr);
    static clang::Expr* BuildOp(clang::Sema& semaRef,
                                clang::BinaryOperatorKind opCode,
                                clang::Expr* L, clang::Expr* R,
                                clang::Scope* S = nullptr);

    clang::ParenExpr* BuildParenExpr(clang::Expr* E);
    static clang::ParenExpr* BuildParenExpr(clang::Sema& semaRef,
                                            clang::Expr* E);

    void BuildNNS(clang::CXXScopeSpec& CSS, clang::DeclContext* DC);

    static void BuildNNS(clang::Sema& semaRef, clang::CXXScopeSpec& CSS,
                         clang::DeclContext* DC);

    clang::ClassTemplateDecl*
    FindBaseTemplateClass(clang::DeclContext* DC,
                          clang::DeclarationNameInfo nameInfo);

    static clang::ClassTemplateDecl*
    FindBaseTemplateClass(clang::Sema& semaRef, clang::DeclContext* DC,
                          clang::DeclarationNameInfo nameInfo);

    static void
    AddSpecialisation(clang::ClassTemplateDecl* baseTemplate,
                      clang::ClassTemplateSpecializationDecl* specialisation);

    clang::ArraySubscriptExpr* BuildBuiltinArraySubscriptExpr(clang::Expr* base,
                                                              clang::Expr* idx);

    static clang::ArraySubscriptExpr*
    BuildBuiltinArraySubscriptExpr(clang::Sema& semaRef, clang::Expr* base,
                                   clang::Expr* idx);

    clang::StringLiteral* BuildStringLiteral(llvm::StringRef str);
    static clang::StringLiteral* BuildStringLiteral(clang::Sema& semaRef,
                                                    llvm::StringRef str);

    static void PrintDecl(clang::Decl* D, llvm::raw_ostream& os);

    /// Function to easily create `FunctionDecl`, `CXXMethodDecl` and
    /// `CXXConstructorDecl` nodes.
    ///
    /// This function handles all the boilerplate code such as handling function
    /// scopes, declaration context, calling relevant sema functions etc. Users
    /// need to pass a function object that should returns the function body AST
    /// node on calling.
    clang::FunctionDecl* BuildFunction(
        clang::DeclContext* DC, clang::DeclarationNameInfo DNI,
        clang::QualType T, ast_helper::ScopeHandler SH,
        std::function<llvm::ArrayRef<clang::ParmVarDecl*>(clang::FunctionDecl*)>
            buildFnParams,
        std::function<clang::Stmt*(clang::FunctionDecl*)> buildFnBody,
        clang::StorageClass S = clang::StorageClass::SC_None,
        bool isInlineSpecified = false);

    static clang::FunctionDecl* BuildFunction(
        clang::Sema& semaRef, clang::DeclContext* DC,
        clang::DeclarationNameInfo DNI, clang::QualType T,
        ast_helper::ScopeHandler SH,
        std::function<llvm::ArrayRef<clang::ParmVarDecl*>(clang::FunctionDecl*)>
            buildFnParams,
        std::function<clang::Stmt*(clang::FunctionDecl*)> buildFnBody,
        clang::StorageClass S = clang::StorageClass::SC_None,
        bool isInlineSpecified = false);

    clang::TemplateArgumentListInfo GetTemplateArgumentListInfo(
        clang::ClassTemplateSpecializationDecl* specialization);

    static clang::TemplateArgumentListInfo GetTemplateArgumentListInfo(
        clang::Sema& semaRef,
        clang::ClassTemplateSpecializationDecl* specialization);
  };
} // namespace clad
#endif