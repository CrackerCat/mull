#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class ScalarValueVisitor
    : public clang::RecursiveASTVisitor<ScalarValueVisitor> {
public:
  explicit ScalarValueVisitor(const VisitorParameters &parameters);
  bool VisitExpr(clang::Expr *expression);

  clang::Expr *foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull