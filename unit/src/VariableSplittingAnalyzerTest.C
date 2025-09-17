#include "VariableSplitting.h"
#include "MooseAST.h"

#include "gtest/gtest.h"

using namespace moose::automatic_weak_form;

namespace
{

VariableSplittingAnalyzer makeAnalyzer(unsigned int fe_order = 1, unsigned int dim = 3)
{
  return VariableSplittingAnalyzer(fe_order, dim);
}

} // namespace

TEST(VariableSplittingAnalyzerTest, LaplacianSplitting)
{
  auto analyzer = makeAnalyzer(1, 3);
  NodePtr u = fieldVariable("u");
  NodePtr expr = laplacian(u);

  auto split_vars = analyzer.generateSplitVariables(expr);
  NodePtr transformed = analyzer.transformExpression(expr, split_vars);
  ASSERT_TRUE(transformed);
  EXPECT_EQ(transformed->toString(), "u_d2");

  ASSERT_EQ(split_vars.size(), 1u);
  ASSERT_TRUE(split_vars.count("u_d2"));

  const auto & sv = split_vars.at("u_d2");
  EXPECT_EQ(sv.original_variable, "u");
  EXPECT_EQ(sv.derivative_order, 2u);
  ASSERT_TRUE(std::holds_alternative<ScalarShape>(sv.shape));
  ASSERT_TRUE(sv.definition);
  EXPECT_EQ(sv.definition->toString(), "laplacian(u)");
  ASSERT_TRUE(sv.constraint_residual);
  EXPECT_EQ(sv.constraint_residual->toString(), "(u_d2 - laplacian(u))");
}

TEST(VariableSplittingAnalyzerTest, TripleDerivativeSplitting)
{
  auto analyzer = makeAnalyzer(1, 3);
  NodePtr u = fieldVariable("u");
  NodePtr expr = div(grad(grad(u)));

  auto split_vars = analyzer.generateSplitVariables(expr);
  NodePtr transformed = analyzer.transformExpression(expr, split_vars);
  ASSERT_TRUE(transformed);
  EXPECT_EQ(transformed->toString(), "u_d3");

  ASSERT_EQ(split_vars.size(), 2u);
  ASSERT_TRUE(split_vars.count("u_d2"));
  ASSERT_TRUE(split_vars.count("u_d3"));

  const auto & u_d2 = split_vars.at("u_d2");
  const auto & u_d3 = split_vars.at("u_d3");

  EXPECT_EQ(u_d2.derivative_order, 2u);
  EXPECT_EQ(u_d3.derivative_order, 3u);
  EXPECT_EQ(u_d2.definition->toString(), "grad(grad(u))");
  EXPECT_EQ(u_d3.definition->toString(), "div(grad(grad(u)))");
  EXPECT_EQ(u_d3.constraint_residual->toString(), "(u_d3 - div(grad(grad(u))))");
}

TEST(VariableSplittingAnalyzerTest, MixedVariableSplitting)
{
  auto analyzer = makeAnalyzer(1, 3);
  NodePtr u = fieldVariable("u");
  NodePtr v = fieldVariable("v");
  NodePtr expr = add(div(grad(grad(u))), laplacian(v));

  auto split_vars = analyzer.generateSplitVariables(expr);
  NodePtr transformed = analyzer.transformExpression(expr, split_vars);
  ASSERT_TRUE(transformed);
  EXPECT_EQ(transformed->toString(), "(u_d3 + v_d2)");

  ASSERT_EQ(split_vars.size(), 3u);
  ASSERT_TRUE(split_vars.count("u_d2"));
  ASSERT_TRUE(split_vars.count("u_d3"));
  ASSERT_TRUE(split_vars.count("v_d2"));

  EXPECT_EQ(split_vars.at("u_d2").definition->toString(), "grad(grad(u))");
  EXPECT_EQ(split_vars.at("u_d3").definition->toString(), "div(grad(grad(u)))");
  EXPECT_EQ(split_vars.at("v_d2").definition->toString(), "laplacian(v)");
}
