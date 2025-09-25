#include "VariableSplitting.h"
#include "MooseAST.h"

#include "gtest/gtest.h"

#include <algorithm>

using namespace moose::automatic_weak_form;

namespace
{

VariableSplittingAnalyzer
makeAnalyzer()
{
  return VariableSplittingAnalyzer(1, 3);
}

std::map<std::string, SplitVariable>
makeFourthOrderSplits()
{
  auto analyzer = makeAnalyzer();
  NodePtr u = fieldVariable("u");
  NodePtr energy = laplacian(laplacian(u));
  return analyzer.generateSplitVariables(energy);
}

unsigned int
componentCount(const Shape & shape)
{
  if (std::holds_alternative<ScalarShape>(shape))
    return 1u;
  if (const auto * vec = std::get_if<VectorShape>(&shape))
    return vec->dim;
  if (const auto * tensor = std::get_if<TensorShape>(&shape))
    return tensor->dim * tensor->dim;
  if (const auto * rank3 = std::get_if<RankThreeShape>(&shape))
    return rank3->dim * rank3->dim * rank3->dim;
  if (const auto * rank4 = std::get_if<RankFourShape>(&shape))
    return rank4->dim1 * rank4->dim2;
  return 1u;
}

unsigned int
expectedDofs(const HigherOrderSplittingStrategy::SplitPlan & plan)
{
  unsigned int total = 1u;
  for (const auto & sv : plan.variables)
    total += componentCount(sv.shape);
  return total;
}

} // namespace

TEST(HigherOrderSplittingStrategyTest, RecursivePlanFormsChain)
{
  auto split_vars = makeFourthOrderSplits();
  ASSERT_EQ(split_vars.size(), 4u);
  ASSERT_TRUE(split_vars.count("u_d1"));
  ASSERT_TRUE(split_vars.count("u_d2"));
  ASSERT_TRUE(split_vars.count("u_d3"));
  ASSERT_TRUE(split_vars.count("u_d4"));

  HigherOrderSplittingStrategy strategy;
  auto plan = strategy.createRecursiveSplitting("u", split_vars, 1);
  strategy.optimizeBandwidth(plan);

  ASSERT_EQ(plan.variables.size(), 4u);
  EXPECT_EQ(plan.variables[0].name, "u_d1");
  EXPECT_EQ(plan.variables[1].name, "u_d2");
  EXPECT_EQ(plan.variables[2].name, "u_d3");
  EXPECT_EQ(plan.variables[3].name, "u_d4");

  std::vector<std::pair<std::string, std::string>> expected_deps = {
      {"u_d1", "u"}, {"u_d2", "u_d1"}, {"u_d3", "u_d2"}, {"u_d4", "u_d3"}};
  EXPECT_EQ(plan.dependencies, expected_deps);

  EXPECT_EQ(plan.total_dofs, expectedDofs(plan));
  EXPECT_LE(plan.bandwidth, 3u);
  EXPECT_EQ(plan.strategy, HigherOrderSplittingStrategy::Strategy::RECURSIVE);
}

TEST(HigherOrderSplittingStrategyTest, DirectPlanDependsOnPrimary)
{
  auto split_vars = makeFourthOrderSplits();
  HigherOrderSplittingStrategy strategy;
  auto plan = strategy.createDirectSplitting("u", split_vars, 1);
  strategy.optimizeBandwidth(plan);

  ASSERT_EQ(plan.variables.size(), 4u);

  std::vector<std::pair<std::string, std::string>> expected_deps = {
      {"u_d1", "u"}, {"u_d2", "u"}, {"u_d3", "u"}, {"u_d4", "u"}};
  EXPECT_EQ(plan.dependencies, expected_deps);

  EXPECT_EQ(plan.total_dofs, expectedDofs(plan));
  EXPECT_EQ(plan.bandwidth, 1u);
  EXPECT_EQ(plan.strategy, HigherOrderSplittingStrategy::Strategy::DIRECT);
}

TEST(HigherOrderSplittingStrategyTest, MixedPlanBlendsChainAndDirect)
{
  auto split_vars = makeFourthOrderSplits();
  HigherOrderSplittingStrategy strategy;
  auto plan = strategy.createMixedSplitting("u", split_vars, 1, 2);
  strategy.optimizeBandwidth(plan);

  ASSERT_EQ(plan.variables.size(), 4u);

  std::vector<std::pair<std::string, std::string>> expected_deps = {
      {"u_d1", "u"}, {"u_d2", "u_d1"}, {"u_d3", "u"}, {"u_d4", "u"}};
  EXPECT_EQ(plan.dependencies, expected_deps);

  EXPECT_EQ(plan.total_dofs, expectedDofs(plan));
  EXPECT_LE(plan.bandwidth, 3u);
  EXPECT_EQ(plan.strategy, HigherOrderSplittingStrategy::Strategy::MIXED);
}

TEST(HigherOrderSplittingStrategyTest, ComputeOptimalUsesProvidedSplits)
{
  auto analyzer = makeAnalyzer();
  NodePtr u = fieldVariable("u");
  NodePtr energy = laplacian(laplacian(u));

  auto split_map = analyzer.generateSplitVariables(energy);
  auto requirements = analyzer.analyzeExpression(energy);

  auto req_it = std::find_if(requirements.begin(), requirements.end(), [](const auto & req) {
    return req.variable_name == "u";
  });
  ASSERT_NE(req_it, requirements.end());

  HigherOrderSplittingStrategy strategy;
  auto plan = strategy.computeOptimalSplitting(
      energy, "u", req_it->max_derivative_order, 1, split_map);

  ASSERT_EQ(plan.variables.size(), 4u);
  EXPECT_EQ(plan.strategy, HigherOrderSplittingStrategy::Strategy::DIRECT);

  std::vector<std::pair<std::string, std::string>> expected_deps = {
      {"u_d1", "u"}, {"u_d2", "u"}, {"u_d3", "u"}, {"u_d4", "u"}};
  EXPECT_EQ(plan.dependencies, expected_deps);

  EXPECT_EQ(plan.bandwidth, 1u);
  EXPECT_EQ(plan.total_dofs, expectedDofs(plan));
}

TEST(HigherOrderSplittingStrategyTest, ComputeOptimalAnalyzesWhenSplitsMissing)
{
  auto analyzer = makeAnalyzer();
  NodePtr u = fieldVariable("u");
  NodePtr energy = laplacian(laplacian(u));

  auto requirements = analyzer.analyzeExpression(energy);
  auto req_it = std::find_if(requirements.begin(), requirements.end(), [](const auto & req) {
    return req.variable_name == "u";
  });
  ASSERT_NE(req_it, requirements.end());

  HigherOrderSplittingStrategy strategy;
  std::map<std::string, SplitVariable> empty;
  auto plan = strategy.computeOptimalSplitting(
      energy, "u", req_it->max_derivative_order, 1, empty);

  ASSERT_EQ(plan.variables.size(), 4u);
  EXPECT_EQ(plan.strategy, HigherOrderSplittingStrategy::Strategy::DIRECT);
  EXPECT_EQ(plan.bandwidth, 1u);
  EXPECT_EQ(plan.total_dofs, expectedDofs(plan));
}
