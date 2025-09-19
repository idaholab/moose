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

  ASSERT_EQ(split_vars.size(), 2u);
  ASSERT_TRUE(split_vars.count("u_d1"));
  ASSERT_TRUE(split_vars.count("u_d2"));

  const auto & u_d1 = split_vars.at("u_d1");
  const auto & u_d2 = split_vars.at("u_d2");

  EXPECT_EQ(u_d1.derivative_order, 1u);
  ASSERT_TRUE(std::holds_alternative<VectorShape>(u_d1.shape));
  EXPECT_EQ(u_d1.definition->toString(), "grad(u)");
  EXPECT_EQ(u_d1.constraint_residual->toString(), "(grad(u) - u_d1)");

  EXPECT_EQ(u_d2.derivative_order, 2u);
  ASSERT_TRUE(std::holds_alternative<ScalarShape>(u_d2.shape));
  EXPECT_EQ(u_d2.definition->toString(), "div(u_d1)");
  EXPECT_EQ(u_d2.constraint_residual->toString(), "(div(u_d1) - u_d2)");

  auto constraints = analyzer.generateConstraintEquations(split_vars);
  ASSERT_EQ(constraints.size(), 2u);
  EXPECT_EQ(constraints[0]->toString(), "(grad(u) - u_d1)");
  EXPECT_EQ(constraints[1]->toString(), "(div(u_d1) - u_d2)");
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

  ASSERT_EQ(split_vars.size(), 3u);
  ASSERT_TRUE(split_vars.count("u_d1"));
  ASSERT_TRUE(split_vars.count("u_d2"));
  ASSERT_TRUE(split_vars.count("u_d3"));

  const auto & u_d1 = split_vars.at("u_d1");
  const auto & u_d2 = split_vars.at("u_d2");
  const auto & u_d3 = split_vars.at("u_d3");

  EXPECT_EQ(u_d1.derivative_order, 1u);
  EXPECT_EQ(u_d1.definition->toString(), "grad(u)");
  EXPECT_EQ(u_d1.constraint_residual->toString(), "(grad(u) - u_d1)");

  EXPECT_EQ(u_d2.derivative_order, 2u);
  EXPECT_EQ(u_d2.definition->toString(), "grad(u_d1)");
  EXPECT_EQ(u_d2.constraint_residual->toString(), "(grad(u_d1) - u_d2)");

  EXPECT_EQ(u_d3.derivative_order, 3u);
  EXPECT_EQ(u_d3.definition->toString(), "div(u_d2)");
  EXPECT_EQ(u_d3.constraint_residual->toString(), "(div(u_d2) - u_d3)");
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

  ASSERT_EQ(split_vars.size(), 5u);
  ASSERT_TRUE(split_vars.count("u_d1"));
  ASSERT_TRUE(split_vars.count("u_d2"));
  ASSERT_TRUE(split_vars.count("u_d3"));
  ASSERT_TRUE(split_vars.count("v_d1"));
  ASSERT_TRUE(split_vars.count("v_d2"));

  EXPECT_EQ(split_vars.at("u_d1").definition->toString(), "grad(u)");
  EXPECT_EQ(split_vars.at("u_d2").definition->toString(), "grad(u_d1)");
  EXPECT_EQ(split_vars.at("u_d3").definition->toString(), "div(u_d2)");
  EXPECT_EQ(split_vars.at("v_d1").definition->toString(), "grad(v)");
  EXPECT_EQ(split_vars.at("v_d2").definition->toString(), "div(v_d1)");
}

TEST(VariableSplittingAnalyzerTest, ComponentTransformation)
{
  auto analyzer = makeAnalyzer(1, 3);
  NodePtr u = fieldVariable("u");
  NodePtr hessian = grad(grad(u));
  NodePtr component = std::make_shared<ComponentNode>(hessian, 0u, 0u, ScalarShape{});

  auto split_vars = analyzer.generateSplitVariables(component);
  NodePtr transformed = analyzer.transformExpression(component, split_vars);
  ASSERT_TRUE(transformed);
  EXPECT_EQ(transformed->toString(), "u_d2[0,0]");

  ASSERT_TRUE(split_vars.count("u_d1"));
  ASSERT_TRUE(split_vars.count("u_d2"));
  EXPECT_EQ(split_vars.at("u_d2").definition->toString(), "grad(u_d1)");
}
