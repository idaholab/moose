#include "gtest/gtest.h"

#include "StringExpressionParser.h"
#include "EvaluationTape.h"

#include <unordered_map>

using namespace moose::automatic_weak_form;

namespace
{
std::unordered_map<std::string, TapeValue>
makeInputs(const RealVectorValue & grad_u, Real u = 0.0)
{
  std::unordered_map<std::string, TapeValue> inputs;
  inputs["u"] = u;
  inputs["u_grad"] = grad_u;
  return inputs;
}
}

TEST(EvaluationTapeTest, GradientDot)
{
  StringExpressionParser parser;
  auto expr = parser.parse("dot(grad(u), grad(u))");

  auto build = buildScalarTape(expr, "u");
  ASSERT_TRUE(build.success);

  RealVectorValue grad_u(1.0, -2.0, 3.0);
  auto inputs = makeInputs(grad_u, 0.5);

  auto result = build.tape.evaluate(inputs);
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(std::holds_alternative<TapeScalar>(*result));

  Real expected = grad_u * grad_u;
  EXPECT_NEAR(std::get<TapeScalar>(*result), expected, 1e-12);
}

TEST(EvaluationTapeTest, ScalarPrefactor)
{
  StringExpressionParser parser;
  auto expr = parser.parse("0.5 * dot(grad(u), grad(u))");

  auto build = buildScalarTape(expr, "u");
  ASSERT_TRUE(build.success);

  RealVectorValue grad_u(0.25, 0.5, 1.0);
  auto inputs = makeInputs(grad_u, 2.0);

  auto result = build.tape.evaluate(inputs);
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(std::holds_alternative<TapeScalar>(*result));

  Real expected = 0.5 * (grad_u * grad_u);
  EXPECT_NEAR(std::get<TapeScalar>(*result), expected, 1e-12);
}

TEST(EvaluationTapeTest, ContractOuterProduct)
{
  StringExpressionParser parser;
  auto expr = parser.parse("contract(outer(grad(u), grad(u)), outer(grad(u), grad(u)))");

  auto build = buildScalarTape(expr, "u");
  ASSERT_TRUE(build.success);

  RealVectorValue grad_u(1.0, 2.0, 3.0);
  auto inputs = makeInputs(grad_u);

  auto result = build.tape.evaluate(inputs);
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(std::holds_alternative<TapeScalar>(*result));

  Real norm_sq = grad_u * grad_u;
  Real expected = norm_sq * norm_sq;
  EXPECT_NEAR(std::get<TapeScalar>(*result), expected, 1e-10);
}
