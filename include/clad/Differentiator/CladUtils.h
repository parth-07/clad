// This file contains utility functions which do not belong anywhere else

#ifndef CLAD_UTILS_CLADUTILS_H
#define CLAD_UTILS_CLADUTILS_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"

#include <string>

namespace clad {
  namespace utils {
    /// If `FD` is an overloaded operator, returns a name, unique for
    /// each operator, that can be used to create valid C++ identifiers.
    /// Otherwise if `FD` is an ordinary function, returns the name of the
    /// function `FD`.
    std::string ComputeEffectiveFnName(const clang::FunctionDecl* FD);

    /// Creates and returns a compound statement having statements as follows:
    /// {`S`, all the statement of `initial` in sequence}    
    clang::CompoundStmt* PrependAndCreateCompoundStmt(clang::ASTContext& C,
                                                      clang::Stmt* initial,
                                                      clang::Stmt* S);

    /// Creates and returns a compound statement having statements as follows:
    /// {all the statements of `initial` in sequence, `S`}
    clang::CompoundStmt* AppendAndCreateCompoundStmt(clang::ASTContext& C,
                                                     clang::Stmt* initial,
                                                     clang::Stmt* S);
  }
}

#endif