#include "WeakFormGenerator.h"
#include "VariableSplitting.h"
#include "MooseAST.h"

#include "gtest/gtest.h"

using namespace moose::automatic_weak_form;

namespace
{

std::map<std::string, NodePtr>
extractDefinitions(const std::map<std::string, SplitVariable> & split_vars)
{
  std::map<std::string, NodePtr> defs;
  for (const auto & [name, sv] : split_vars)
    defs[name] = sv.definition;
  return defs;
}

} // namespace

TEST(WeakFormGeneratorSplitTest, LaplacianEnergyMatchesBaseline)
{
  VariableSplittingAnalyzer analyzer(1, 3);

  NodePtr u = fieldVariable("u");
  NodePtr energy = multiply(laplacian(u), laplacian(u));

  WeakFormGenerator baseline_generator(3);
  auto baseline = baseline_generator.computeContributions(energy, "u");
  ASSERT_TRUE(baseline.total_residual);

  auto split_vars = analyzer.generateSplitVariables(energy);
  auto split_defs = extractDefinitions(split_vars);
  NodePtr transformed = analyzer.transformExpression(energy, split_vars);

  WeakFormGenerator split_generator(3);
  split_generator.setSplitDefinitions(split_defs);
  auto split = split_generator.computeContributions(transformed, "u");
  ASSERT_TRUE(split.total_residual);

  EXPECT_EQ(split.total_residual->toString(), baseline.total_residual->toString());
}

TEST(WeakFormGeneratorSplitTest, SplitVariableCarriesDerivativeInformation)
{
  VariableSplittingAnalyzer analyzer(1, 3);

  NodePtr u = fieldVariable("u");
  NodePtr expr = laplacian(u);

  auto split_vars = analyzer.generateSplitVariables(expr);
  auto split_defs = extractDefinitions(split_vars);

  ASSERT_TRUE(split_vars.count("u_d2"));
  auto split_it = split_vars.find("u_d2");

  DifferentiationVisitor dv("u", &split_defs);
  auto diff = dv.differentiate(fieldVariable(split_it->first));

  EXPECT_TRUE(diff.hasOrder(2));
  ASSERT_TRUE(diff.getCoefficient(2));
  EXPECT_EQ(diff.getCoefficient(2)->toString(), constant(1.0)->toString());
  EXPECT_FALSE(diff.hasOrder(0));
  EXPECT_FALSE(diff.hasOrder(1));
}

