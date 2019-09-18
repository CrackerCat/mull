#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/AndOrReplacementMutator.h"
#include "mull/Mutators/MathDivMutator.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/ReplaceAssignmentMutator.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"
#include "mull/Program/Program.h"
#include "mull/Testee.h"
#include "mull/MutationPoint.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <mull/Mutators/CXX/RelationalMutators.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(MutationPoint, MathDivMutator_applyMutation) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_math_div_module_bc_path(), context);

  MathDivMutator mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(), bitcode->getModule()->getFunction("math_div"));

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  MutationPointAddress address = mutationPoint->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(
      mutationPoint->getOriginalFunction());

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Mul, mutatedInstruction.getOpcode());
}

TEST(MutationPoint, NegateConditionMutator_applyMutation) {
  LLVMContext context;
  BitcodeLoader loader;

  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_negate_condition_testee_bc_path(), context);

  cxx::LessThanToGreaterOrEqual mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(), bitcode->getModule()->getFunction("max"));

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  ASSERT_TRUE(isa<CmpInst>(mutationPoint->getOriginalValue()));
  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(
      mutationPoint->getOriginalFunction());
  auto mutatedCmpInstruction = cast<CmpInst>(&mutatedInstruction);
  ASSERT_EQ(mutatedCmpInstruction->getPredicate(),
            CmpInst::Predicate::ICMP_SGE);
}

TEST(MutationPoint, AndOrReplacementMutator_applyMutation) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_and_or_replacement_module_bc_path(), context);

  AndOrReplacementMutator mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(),
      bitcode->getModule()->getFunction("testee_AND_operator_2branches"));

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(
      mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<BranchInst>(&mutatedInstruction));
}

TEST(MutationPoint, ScalarValueMutator_applyMutation) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_scalar_value_module_bc_path(), context);

  ScalarValueMutator mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(), bitcode->getModule()->getFunction("scalar_value"));

  ASSERT_EQ(4U, mutationPoints.size());

  MutationPoint *mutationPoint1 = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint1->getAddress();
  ASSERT_TRUE(isa<StoreInst>(mutationPoint1->getOriginalValue()));

  MutationPoint *mutationPoint2 = mutationPoints[1];
  MutationPointAddress mutationPointAddress2 = mutationPoint2->getAddress();
  ASSERT_TRUE(isa<StoreInst>(mutationPoint2->getOriginalValue()));

  MutationPoint *mutationPoint3 = mutationPoints[2];
  MutationPointAddress mutationPointAddress3 = mutationPoint3->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint3->getOriginalValue()));

  MutationPoint *mutationPoint4 = mutationPoints[3];
  MutationPointAddress mutationPointAddress4 = mutationPoint4->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint4->getOriginalValue()));

  mutationPoint1->setMutatedFunction(mutationPoint1->getOriginalFunction());
  mutationPoint1->applyMutation();

  auto &mutatedInstruction = mutationPoint1->getAddress().findInstruction(
      mutationPoint1->getOriginalFunction());
  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

TEST(MutationPoint, ReplaceCallMutator_applyMutation) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_replace_call_module_bc_path(), context);

  ReplaceCallMutator mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(), bitcode->getModule()->getFunction("replace_call"));

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint->getAddress();
  ASSERT_TRUE(isa<CallInst>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(
      mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
}

TEST(MutationPoint, ReplaceAssignmentMutator_applyMutation) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_replace_assignment_module_bc_path(), context);

  ReplaceAssignmentMutator mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(), bitcode->getModule()->getFunction("replace_assignment"));

  EXPECT_EQ(2U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints[0];
  EXPECT_TRUE(isa<StoreInst>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(
      mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

TEST(MutationPoint, OriginalValuePresent) {
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_replace_assignment_module_bc_path(), context);

  ReplaceAssignmentMutator mutator;
  auto mutationPoints = mutator.getMutations(
      bitcode.get(), bitcode->getModule()->getFunction("replace_assignment"));

  ASSERT_EQ(2U, mutationPoints.size());

  std::vector<std::string> mutantRepresentations;
  for (auto *mutation : mutationPoints) {
    std::string s;
    llvm::raw_string_ostream stream(s);
    mutation->getOriginalValue()->print(stream);
    mutantRepresentations.push_back(s);
  }

  for (auto *mutation : mutationPoints) {
    bitcode->addMutation(mutation);
  }
  bitcode->prepareMutations();
  for (auto *mutation : mutationPoints) {
    mutation->applyMutation();
  }

  for (auto i = 0; i < mutationPoints.size(); i++) {
    auto mutation = mutationPoints[i];
    std::string s;
    llvm::raw_string_ostream stream(s);
    mutation->getOriginalValue()->print(stream);
    ASSERT_EQ(s, mutantRepresentations[i]);
  }
}
