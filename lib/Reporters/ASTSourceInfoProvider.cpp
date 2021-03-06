#include "mull/Reporters/ASTSourceInfoProvider.h"

#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/Logger.h"

#include <clang/AST/Expr.h>
#include <clang/Lex/Lexer.h>

using namespace mull;

ASTSourceInfoProvider::ASTSourceInfoProvider(ASTStorage &astStorage)
    : astStorage(astStorage) {}

MutationPointSourceInfo
ASTSourceInfoProvider::getSourceInfo(MutationPoint *mutationPoint) {
  MutationPointSourceInfo info;
  clang::SourceRange sourceRange;

  clang::Expr *mutantASTNode = astStorage.getMutantASTNode(mutationPoint);
  ThreadSafeASTUnit *astUnit = astStorage.findAST(mutationPoint);

  if (mutantASTNode == nullptr) {
    mull::Logger::warn()
        << "error: cannot find an AST node for mutation point\n";
    return info;
  }
  if (astUnit == nullptr) {
    mull::Logger::warn()
        << "error: cannot find an AST unit for mutation point\n";
    return info;
  }

  sourceRange = mutantASTNode->getSourceRange();

  clang::ASTContext &astContext = astUnit->getASTContext();

  clang::SourceManager &sourceManager = astUnit->getSourceManager();

  clang::SourceLocation sourceLocationBegin = sourceRange.getBegin();
  clang::SourceLocation sourceLocationEnd = sourceRange.getEnd();

  /// Clang AST: how to get more precise debug information in certain cases?
  /// http://clang-developers.42468.n3.nabble.com/Clang-AST-how-to-get-more-precise-debug-information-in-certain-cases-td4065195.html
  /// https://stackoverflow.com/questions/11083066/getting-the-source-behind-clangs-ast
  clang::SourceLocation sourceLocationEndActual =
      clang::Lexer::getLocForEndOfToken(sourceLocationEnd, 0, sourceManager,
                                        astContext.getLangOpts());

  info.beginColumn =
      sourceManager.getExpansionColumnNumber(sourceLocationBegin);
  info.beginLine =
      sourceManager.getExpansionLineNumber(sourceLocationBegin, nullptr);
  info.endColumn =
      sourceManager.getExpansionColumnNumber(sourceLocationEndActual);
  info.endLine =
      sourceManager.getExpansionLineNumber(sourceLocationEndActual, nullptr);

  return info;
}
