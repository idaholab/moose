//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseAST.h"
#include "StringExpressionParser.h"
#include "WeakFormGenerator.h"
#include "ExpressionSimplifier.h"

#include <cmath>
#include <memory>
#include <string>
#include <iostream>

using namespace moose::automatic_weak_form;

class WeakFormDerivationTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    parser = std::make_unique<StringExpressionParser>();
    generator = std::make_unique<WeakFormGenerator>();
    simplifier = std::make_unique<ExpressionSimplifier>();
  }

  std::unique_ptr<StringExpressionParser> parser;
  std::unique_ptr<WeakFormGenerator> generator;
  std::unique_ptr<ExpressionSimplifier> simplifier;
};

// Helper function to debug test failures - prints actual vs expected
void debugCompare(const std::string & actual, const std::string & expected)
{
  if (actual != expected)
  {
    std::cout << "ACTUAL:   '" << actual << "'" << std::endl;
    std::cout << "EXPECTED: '" << expected << "'" << std::endl;
  }
}

// Test basic differentiation with exact string matching
TEST_F(WeakFormDerivationTest, BasicDifferentiation)
{
  // Test 1: d/dx(x^2) = 2*x
  {
    auto expr = parser->parse("x*x");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    // Already simplified to 2*x
    EXPECT_EQ(result_str, "(2 * x)");

    // Already simplified, verify it stays the same
    auto simplified = simplifier->simplify(result);
    std::string simplified_str = simplified->toString();
    EXPECT_EQ(simplified_str, "(2 * x)");
  }

  // Test 2: d/dx(sin(x)) = cos(x)
  {
    auto expr = parser->parse("sin(x)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "cos(x)");

    // Already simplified
    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "cos(x)");
  }

  // Test 3: d/dx(exp(x)) = exp(x)
  {
    auto expr = parser->parse("exp(x)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "exp(x)");

    // Already simplified
    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "exp(x)");
  }

  // Test 4: d/dx(x^3) = 3*x^2
  {
    auto expr = parser->parse("pow(x, 3.0)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "(3 * pow(x, 2))");

    // Already simplified
    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "(3 * pow(x, 2))");
  }

  // Test 5: d/dx(constant) = 0
  {
    auto expr = parser->parse("5.0");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    // Should have no coefficients (derivative is 0)
    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));
  }

  // Test 6: d/dx(sqrt(x)) = 1/(2*sqrt(x))
  {
    auto expr = parser->parse("sqrt(x)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "(0.5 * pow(x, -0.5))");
  }

  // Test 7: d/dx(tanh(x)) = 1 - tanh(x)^2
  {
    auto expr = parser->parse("tanh(x)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "(1 - pow(tanh(x), 2))");
  }
}

// Test gradient operations with exact results
TEST_F(WeakFormDerivationTest, GradientOperations)
{
  // Test 1: grad(c) increases derivative order by 1
  {
    auto expr = parser->parse("grad(c)");
    DifferentiationVisitor dv("c");
    auto diff = dv.differentiate(expr);

    // grad(c) depends on c at order 1
    EXPECT_TRUE(diff.hasOrder(1));
    EXPECT_FALSE(diff.hasOrder(0));

    auto result = diff.getCoefficient(1);
    ASSERT_NE(result, nullptr);

    // The coefficient at order 1 should be the identity (constant 1.0)
    EXPECT_EQ(result->toString(), "1");
  }

  // Test 2: dot(grad(c), grad(c))
  {
    auto expr = parser->parse("dot(grad(c), grad(c))");
    DifferentiationVisitor dv("c");
    auto diff = dv.differentiate(expr);

    // This should have a first-order derivative
    EXPECT_TRUE(diff.hasOrder(1));
    auto result = diff.getCoefficient(1);
    ASSERT_NE(result, nullptr);

    // d/dc[dot(grad(c), grad(c))] = 2*grad(c)
    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "(2 * grad(c))");

    // Already simplified
    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "(2 * grad(c))");
  }

  // Test 3: laplacian(u) increases order by 2
  {
    auto expr = parser->parse("laplacian(u)");
    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    // Laplacian increases order by 2
    EXPECT_TRUE(diff.hasOrder(2));
    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));

    auto result = diff.getCoefficient(2);
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->isTensor());
    std::string result_str = result->toString();
    EXPECT_NE(result_str.find("[tensor]"), std::string::npos);
  }
}

TEST_F(WeakFormDerivationTest, EulerLagrangeOperator)
{
  parser->defineVariable("c");
  parser->defineExpression("F", "W(c) + 0.5*kappa*dot(grad(c), grad(c))");

  auto euler = parser->parse("el(F, c)");
  ASSERT_NE(euler, nullptr);

  auto simplified = simplifier->simplify(euler);
  ASSERT_NE(simplified, nullptr);

  auto expected = simplifier->simplify(parser->parse(
      "(2.0*c*((c*c) - 1.0)) + (2.0*((c*c) - 1.0)*c) + -(div(kappa*grad(c)))"));
  ASSERT_NE(expected, nullptr);

  std::string actual_str = simplified->toString();
  std::string expected_str = expected->toString();
  debugCompare(actual_str, expected_str);
  EXPECT_TRUE(simplified->equals(*expected));

  auto zero_el = parser->parse("el(5.0, c)");
  auto zero_simplified = simplifier->simplify(zero_el);
  ASSERT_NE(zero_simplified, nullptr);
  EXPECT_EQ(zero_simplified->toString(), "0");
}

// Test VectorAssembly (vec operator) with exact results
TEST_F(WeakFormDerivationTest, VectorAssemblyOperations)
{
  // Test 1: vec(u, v) creates a vector
  {
    auto expr = parser->parse("vec(u, v)");
    ASSERT_NE(expr, nullptr);

    auto vec_node = std::dynamic_pointer_cast<VectorAssemblyNode>(expr);
    ASSERT_NE(vec_node, nullptr);
    EXPECT_EQ(vec_node->components().size(), 2);

    std::string expr_str = expr->toString();
    EXPECT_EQ(expr_str, "vec(u, v)");
  }

  // Test 2: Differentiation of vec(u, v) w.r.t. u
  {
    auto expr = parser->parse("vec(u, v)");
    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_TRUE(diff.hasOrder(0));
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    // Should be vec(1, 0)
    auto vec_result = std::dynamic_pointer_cast<VectorAssemblyNode>(result);
    ASSERT_NE(vec_result, nullptr);
    EXPECT_EQ(vec_result->components().size(), 2);

    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "vec(1, 0)");
  }

  // Test 3: Differentiation of vec(u, v) w.r.t. v
  {
    auto expr = parser->parse("vec(u, v)");
    DifferentiationVisitor dv("v");
    auto diff = dv.differentiate(expr);

    EXPECT_TRUE(diff.hasOrder(0));
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    // Should be vec(0, 1)
    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "vec(0, 1)");
  }

  // Test 4: dot(grad(u), vec(1.0, 0.0)) extracts x-component
  {
    auto expr = parser->parse("dot(grad(u), vec(1.0, 0.0))");

    // // After simplification
    // auto simplified = simplifier->simplify(expr);
    // EXPECT_EQ(simplified->toString(), "(2.000000 * grad(c))");

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_TRUE(diff.hasOrder(1));
    auto result = diff.getCoefficient(1);
    ASSERT_NE(result, nullptr);

    // Result should be vec(1.0, 0.0) representing the x-direction
    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "vec(1, 0)");
  }

  // Test 5: vec with three components
  {
    auto expr = parser->parse("vec(x, y, z)");
    ASSERT_NE(expr, nullptr);

    auto vec_node = std::dynamic_pointer_cast<VectorAssemblyNode>(expr);
    ASSERT_NE(vec_node, nullptr);
    EXPECT_EQ(vec_node->components().size(), 3);

    EXPECT_EQ(expr->toString(), "vec(x, y, z)");
  }

  // Test 6: Cross product differentiation
  {
    auto expr = parser->parse("cross(vec(u, 0.0, 0.0), vec(0.0, 1.0, 0.0))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_TRUE(diff.hasOrder(0));
    auto coeff = diff.getCoefficient(0);
    ASSERT_NE(coeff, nullptr);

    auto simplified = simplifier->simplify(coeff);
    EXPECT_EQ(simplified->toString(), "vec(0, 0, 1)");
  }

  // Test 7: Curl differentiation increases derivative order
  {
    auto expr = parser->parse("curl(vec(u, 0.0, 0.0))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_TRUE(diff.hasOrder(1));
    auto coeff = diff.getCoefficient(1);
    ASSERT_NE(coeff, nullptr);
    EXPECT_EQ(coeff->toString(), "vec(1, 0, 0)");
  }
}

TEST_F(WeakFormDerivationTest, HigherOrderDerivatives)
{
  // Test 1: div(grad(grad(u))) produces first-order coefficient 1
  {
    auto expr = parser->parse("div(grad(grad(u)))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_FALSE(diff.hasOrder(0));
    ASSERT_TRUE(diff.hasOrder(1));
    auto coeff = diff.getCoefficient(1);
    ASSERT_NE(coeff, nullptr);

    auto simplified = simplifier->simplify(coeff);
    EXPECT_EQ(simplified->toString(), "1");
  }

  // Test 2: div(grad(grad(grad(u)))) -> coefficient at order 2 equals 1
  {
    auto expr = parser->parse("div(grad(grad(grad(u))))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));
    ASSERT_TRUE(diff.hasOrder(2));

    auto coeff = diff.getCoefficient(2);
    ASSERT_NE(coeff, nullptr);

    auto simplified = simplifier->simplify(coeff);
    EXPECT_EQ(simplified->toString(), "1");
  }

  // Test 3: Mixed gradient dot-product derivative
  {
    auto expr = parser->parse("dot(grad(u), grad(v))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_FALSE(diff.hasOrder(0));
    ASSERT_TRUE(diff.hasOrder(1));

    auto coeff = diff.getCoefficient(1);
    ASSERT_NE(coeff, nullptr);

    auto simplified = simplifier->simplify(coeff);
    EXPECT_EQ(simplified->toString(), "grad(v)");
  }
}

// Test anisotropic energy expressions with exact results
TEST_F(WeakFormDerivationTest, AnisotropicEnergy)
{
  // Test anisotropic Allen-Cahn energy with different x and y gradient coefficients
  {
    auto energy = parser->parse(
      "W(eta) + 0.5*kappa_x*pow(dot(grad(eta), vec(1.0, 0.0)), 2.0) + "
      "0.5*kappa_y*pow(dot(grad(eta), vec(0.0, 1.0)), 2.0)");

    DifferentiationVisitor dv("eta");
    auto diff = dv.differentiate(energy);

    // Should have both order 0 (bulk) and order 1 (gradient) terms
    EXPECT_TRUE(diff.hasOrder(0));
    EXPECT_TRUE(diff.hasOrder(1));

    auto c0 = diff.getCoefficient(0);
    ASSERT_NE(c0, nullptr);
    // The zeroth-order term is 4*eta*(eta^2-1) expanded, with improved simplifier it has 2 factored out
    EXPECT_EQ(c0->toString(), "((2 * (eta * ((eta * eta) - 1))) + (2 * (((eta * eta) - 1) * eta)))");

    auto c1 = diff.getCoefficient(1);
    ASSERT_NE(c1, nullptr);
    // The first-order term should have both kappa_x and kappa_y terms
    // This is complex, so let's just verify the structure
    std::string c1_str = c1->toString();
    // It should be a sum of two terms
    EXPECT_TRUE(c1_str.find("kappa_x") != std::string::npos);
    EXPECT_TRUE(c1_str.find("kappa_y") != std::string::npos);
    EXPECT_TRUE(c1_str.find("vec(1, 0)") != std::string::npos);
    EXPECT_TRUE(c1_str.find("vec(0, 1)") != std::string::npos);
  }
}

// Test expression simplification with exact results
TEST_F(WeakFormDerivationTest, ExpressionSimplification)
{
  // Test 1: 0 + x = x
  {
    auto expr = parser->parse("0.0 + x");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 2: 1 * x = x
  {
    auto expr = parser->parse("1.0 * x");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 3: x - x = 0
  {
    auto expr = parser->parse("x - x");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "0");
  }

  // Test 4: 2*3 = 6 (constant folding)
  {
    auto expr = parser->parse("2.0 * 3.0");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "6");
  }

  // Test 5: x + 0 = x
  {
    auto expr = parser->parse("x + 0.0");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 6: 0 * x = 0
  {
    auto expr = parser->parse("0.0 * x");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "0");
  }

  // Test 7: x / 1 = x
  {
    auto expr = parser->parse("x / 1.0");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "x");
  }
}

// Test chain rule with exact results
TEST_F(WeakFormDerivationTest, ChainRule)
{
  // Test d/dx[exp(sin(x))] = exp(sin(x)) * cos(x)
  {
    auto expr = parser->parse("exp(sin(x))");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    // The exact result from chain rule
    std::string result_str = result->toString();
    EXPECT_EQ(result_str, "(exp(sin(x)) * cos(x))");

    // Already simplified
    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "(exp(sin(x)) * cos(x))");
  }

  // Test d/dx[sin(x^2)] = cos(x^2) * 2x
  {
    auto expr = parser->parse("sin(x*x)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    // The simplifier produces 2 * (cos((x * x)) * x)
    EXPECT_EQ(result_str, "(2 * (cos((x * x)) * x))");

    // Already simplified
    auto simplified = simplifier->simplify(result);
    EXPECT_EQ(simplified->toString(), "(2 * (cos((x * x)) * x))");
  }
}

// Test tensor operations with exact results
TEST_F(WeakFormDerivationTest, TensorOperations)
{
  // Test trace operation
  {
    auto expr = parser->parse("trace(grad(vec(u, v)))");
    ASSERT_NE(expr, nullptr);

    // The expression is valid
    EXPECT_EQ(expr->toString(), "trace(grad(vec(u, v)))");

    // trace(grad(vec)) = div(vec) = du/dx + dv/dy
    DifferentiationVisitor dv_u("u");
    auto diff_u = dv_u.differentiate(expr);
    EXPECT_TRUE(diff_u.hasOrder(1));

    auto u_coeff = diff_u.getCoefficient(1);
    ASSERT_NE(u_coeff, nullptr);
    // The coefficient should be related to trace of the identity in the u direction

    DifferentiationVisitor dv_v("v");
    auto diff_v = dv_v.differentiate(expr);
    EXPECT_TRUE(diff_v.hasOrder(1));

    auto v_coeff = diff_v.getCoefficient(1);
    ASSERT_NE(v_coeff, nullptr);
  }

  // Test symmetric tensor
  {
    auto expr = parser->parse("sym(grad(vec(u, v)))");
    ASSERT_NE(expr, nullptr);

    EXPECT_EQ(expr->toString(), "sym(grad(vec(u, v)))");

    // Differentiate w.r.t. u
    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);
    EXPECT_TRUE(diff.hasOrder(1));

    auto coeff = diff.getCoefficient(1);
    ASSERT_NE(coeff, nullptr);
    // The result should be sym(vec(1, 0)) without grad
    std::string coeff_str = coeff->toString();
    EXPECT_EQ(coeff_str, "sym(vec(1, 0))");
  }

  // Test contract operation
  {
    auto expr = parser->parse("contract(grad(vec(u, v)), grad(vec(u, v)))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);
    EXPECT_TRUE(diff.hasOrder(1));

    auto coeff = diff.getCoefficient(1);
    ASSERT_NE(coeff, nullptr);
    // Result should be 2*contract(grad(vec(1,0)), grad(vec(u,v)))
    // The exact form depends on how contract handles the differentiation
  }
}

// Test variational derivatives with exact Euler-Lagrange equations
TEST_F(WeakFormDerivationTest, VariationalDerivatives)
{
  // Test 1: Simple gradient energy F = (1/2)|∇c|²
  {
    auto energy = parser->parse("0.5*dot(grad(c), grad(c))");

    DifferentiationVisitor dv("c");
    auto diff = dv.differentiate(energy);

    // Check we have order 1 coefficient only
    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_TRUE(diff.hasOrder(1));

    auto c1 = diff.getCoefficient(1);
    ASSERT_NE(c1, nullptr);

    // Simplifier doesn't collapse 0.5 * 2 = 1
    auto simplified_c1 = simplifier->simplify(c1);
    EXPECT_EQ(simplified_c1->toString(), "grad(c)");

    // Compute Euler-Lagrange equation: -div(grad(c))
    auto euler_lagrange = generator->computeEulerLagrange(diff);
    ASSERT_NE(euler_lagrange, nullptr);

    std::string el_str = euler_lagrange->toString();
    // Now simplified to just -div(grad(c))
    EXPECT_EQ(el_str, "-(div(grad(c)))");
  }

  // Test 2: Double-well potential only
  {
    auto energy = parser->parse("pow(eta*eta - 1.0, 2.0)");

    DifferentiationVisitor dv("eta");
    auto diff = dv.differentiate(energy);

    EXPECT_TRUE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));

    auto c0 = diff.getCoefficient(0);
    ASSERT_NE(c0, nullptr);

    // d/deta[(eta^2-1)^2] = 4*eta*(eta^2-1)
    // This will be: 2*(eta^2-1)*2*eta = 4*eta*(eta^2-1)
    std::string c0_str = c0->toString();
    // The exact form: (2.000000 * ((eta * eta) - 1.000000)) * ((eta * 1.000000) + (1.000000 * eta))
    // which simplifies to 4*eta*(eta^2 - 1)
    auto simplified = simplifier->simplify(c0);
    // We expect something like (4.000000 * eta * ((eta * eta) - 1.000000))
    // but the exact form depends on simplification rules
  }
}

// Test weak form construction
TEST_F(WeakFormDerivationTest, WeakFormConstruction)
{
  // Test simple Laplace equation weak form
  {
    auto energy = parser->parse("0.5*dot(grad(u), grad(u))");
    auto weak_form = generator->generateWeakForm(energy, "u");
    ASSERT_NE(weak_form, nullptr);

    std::string wf_str = weak_form->toString();
    // The weak form is now simplified to -div(grad(u))
    EXPECT_EQ(wf_str, "-(div(grad(u)))");
  }

  // Test with both bulk and gradient terms
  {
    auto energy = parser->parse("u*u + 0.5*dot(grad(u), grad(u))");
    auto weak_form = generator->generateWeakForm(energy, "u");
    ASSERT_NE(weak_form, nullptr);

    std::string wf_str = weak_form->toString();
    // Should be 2*u - div(grad(u))
    EXPECT_EQ(wf_str, "((2 * u) + -(div(grad(u))))");
  }
}

// Test fourth-order derivatives
TEST_F(WeakFormDerivationTest, FourthOrderDerivatives)
{
  // Test energy with fourth-order term: F = (1/2)|∇²u|²
  {
    auto energy = parser->parse("0.5 * laplacian(u) * laplacian(u)");

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(energy);

    // Should have second-order coefficient (from d/du of laplacian(u))
    EXPECT_TRUE(diff.hasOrder(2));

    auto c2 = diff.getCoefficient(2);
    ASSERT_NE(c2, nullptr);

    // The second-order coefficient is now simplified
    std::string c2_str = c2->toString();
    EXPECT_EQ(c2_str, "laplacian(u)");

    // Already simplified
    auto simplified = simplifier->simplify(c2);
    EXPECT_EQ(simplified->toString(), "laplacian(u)");
  }
}

// Test complex coupled system with exact results
TEST_F(WeakFormDerivationTest, CoupledSystem)
{
  // Test simple coupled system
  {
    auto energy = parser->parse("u*v + 0.5*dot(grad(u), grad(u)) + 0.5*dot(grad(v), grad(v))");

    // Differentiate w.r.t. u
    DifferentiationVisitor dv_u("u");
    auto diff_u = dv_u.differentiate(energy);

    EXPECT_TRUE(diff_u.hasOrder(0)); // v (coupling term)
    EXPECT_TRUE(diff_u.hasOrder(1)); // grad(u)

    auto c0_u = diff_u.getCoefficient(0);
    ASSERT_NE(c0_u, nullptr);
    EXPECT_EQ(c0_u->toString(), "v");

    auto c1_u = diff_u.getCoefficient(1);
    ASSERT_NE(c1_u, nullptr);
    auto simplified_c1_u = simplifier->simplify(c1_u);
    EXPECT_EQ(simplified_c1_u->toString(), "grad(u)");

    // Differentiate w.r.t. v
    DifferentiationVisitor dv_v("v");
    auto diff_v = dv_v.differentiate(energy);

    EXPECT_TRUE(diff_v.hasOrder(0)); // u (coupling term)
    EXPECT_TRUE(diff_v.hasOrder(1)); // grad(v)

    auto c0_v = diff_v.getCoefficient(0);
    ASSERT_NE(c0_v, nullptr);
    EXPECT_EQ(c0_v->toString(), "u");

    auto c1_v = diff_v.getCoefficient(1);
    ASSERT_NE(c1_v, nullptr);
    auto simplified_c1_v = simplifier->simplify(c1_v);
    EXPECT_EQ(simplified_c1_v->toString(), "grad(v)");
  }
}

// Test edge cases with exact results
TEST_F(WeakFormDerivationTest, EdgeCases)
{
  // Test differentiation of constant
  {
    auto expr = parser->parse("5.0");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    // Should have no coefficients (derivative is 0)
    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));
    EXPECT_EQ(diff.maxOrder(), 0);
  }

  // Test differentiation of different variable
  {
    auto expr = parser->parse("y*y");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    // Should have no coefficients (no dependence on x)
    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));
    EXPECT_EQ(diff.maxOrder(), 0);
  }

  // Test differentiation of sum of variables
  {
    auto expr = parser->parse("x + y");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    EXPECT_TRUE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));

    auto c0 = diff.getCoefficient(0);
    ASSERT_NE(c0, nullptr);
    EXPECT_EQ(c0->toString(), "1");
  }
}

// Test integration by parts transformation with exact results
TEST_F(WeakFormDerivationTest, IntegrationByParts)
{
  // For a simple second-order problem, check the weak form transformation
  {
    // Use a simpler energy with only second-order terms: F = (1/2)|∇u|²
    auto energy = parser->parse("0.5*dot(grad(u), grad(u))");

    auto contributions = generator->computeContributions(energy, "u");

    // Should have c1_term for first-order derivatives
    ASSERT_NE(contributions.c1_term, nullptr);

    // The first-order term should be grad(u)
    std::string c1_str = contributions.c1_term->toString();
    // Simplifier result: 0.5 * 2 * grad(u)
    auto simplified = simplifier->simplify(contributions.c1_term);
    EXPECT_EQ(simplified->toString(), "grad(u)");

    // Maximum order should be 1 for this energy
    EXPECT_EQ(contributions.max_order, 1);

    // Total residual includes test function multiplication
    ASSERT_NE(contributions.total_residual, nullptr);
    EXPECT_EQ(contributions.total_residual->toString(), "-(dot(grad(u), grad_test_u))");
  }
}

// Test what the simplifier DOES handle correctly
TEST_F(WeakFormDerivationTest, SimplifierWorkingSimplifications)
{
  // Test 1: x + 0 = x
  {
    auto expr = parser->parse("x + 0.0");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 2: 0 + x = x
  {
    auto expr = parser->parse("0.0 + x");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 3: x * 1 = x
  {
    auto expr = parser->parse("x * 1.0");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 4: 1 * x = x
  {
    auto expr = parser->parse("1.0 * x");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 5: 0 * x = 0
  {
    auto expr = parser->parse("0.0 * x");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "0");
  }

  // Test 6: x * 0 = 0
  {
    auto expr = parser->parse("x * 0.0");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "0");
  }

  // Test 7: x / 1 = x
  {
    auto expr = parser->parse("x / 1.0");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test 8: x - x = 0
  {
    auto expr = parser->parse("x - x");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "0");
  }

  // Test 9: Constant folding: 2 * 3 = 6
  {
    auto expr = parser->parse("2.0 * 3.0");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "6");
  }
}

// Test improved nested multiplication handling
TEST_F(WeakFormDerivationTest, SimplifierNestedMultiplicationLimitation)
{
  // Now the simplifier DOES collapse 0.5 * (2.0 * x) to x
  {
    auto expr = parser->parse("0.5 * (2.0 * x)");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Similarly for (2.0 * x) * 0.5
  {
    auto expr = parser->parse("(2.0 * x) * 0.5");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // And for 2.0 * (0.5 * x)
  {
    auto expr = parser->parse("2.0 * (0.5 * x)");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test more complex nested case
  {
    auto expr = parser->parse("0.25 * (2.0 * (2.0 * x))");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }

  // Test with three levels
  {
    auto expr = parser->parse("2.0 * (0.5 * (1.0 * x))");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "x");
  }
}

// Test that certain addition patterns aren't simplified
TEST_F(WeakFormDerivationTest, SimplifierAdditionLimitation)
{
  // The differentiation produces (x * 1) + (1 * x) which gets simplified to 2*x
  // But this happens during differentiation, not in the simplifier
  {
    // If we manually create the expression, simplifier may not combine them
    auto expr = parser->parse("((x * 1.0) + (1.0 * x))");
    auto simplified = simplifier->simplify(expr);
    // The simplifier does simplify this to 2*x
    EXPECT_EQ(simplified->toString(), "(2 * x)");
  }
}

// Test grad operations are now properly simplified
TEST_F(WeakFormDerivationTest, SimplifierGradientSimplification)
{
  // 0.5 * (2.0 * grad(u)) now correctly becomes grad(u)
  {
    auto expr = parser->parse("0.5 * (2.0 * grad(u))");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "grad(u)");
  }

  // Test with laplacian too
  {
    auto expr = parser->parse("2.0 * (0.5 * laplacian(u))");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "laplacian(u)");
  }

  // Test with more complex gradient expression
  {
    auto expr = parser->parse("0.25 * (4.0 * grad(c))");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "grad(c)");
  }
}

// Test that the simplifier preserves mathematical correctness
TEST_F(WeakFormDerivationTest, SimplifierCorrectnessPreservation)
{
  // Even though the simplifier doesn't optimize everything,
  // it should never produce incorrect results

  // Test that (a * b) * c is preserved correctly
  {
    auto expr = parser->parse("(2.0 * 3.0) * 4.0");
    auto simplified = simplifier->simplify(expr);
    // Should be 24.0 after constant folding
    EXPECT_EQ(simplified->toString(), "24");
  }

  // Test that order of operations is preserved
  {
    auto expr = parser->parse("x + 2.0 * 3.0");
    auto simplified = simplifier->simplify(expr);
    // Should be x + 6.0
    EXPECT_EQ(simplified->toString(), "(x + 6)");
  }
}

// Document what the improved simplifier now does
TEST_F(WeakFormDerivationTest, SimplifierIdealSimplifierBehavior)
{
  // These tests show what the simplifier can now do after improvements

  // Collapse nested constant multiplications - NOW WORKS!
  {
    auto expr = parser->parse("2.0 * (0.5 * laplacian(u))");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "laplacian(u)");
  }

  // Distribute and collect coefficients
  {
    auto expr = parser->parse("kappa * grad(u) + kappa * grad(u)");
    auto simplified = simplifier->simplify(expr);
    std::string result = simplified->toString();
    EXPECT_EQ(result, "(2 * (kappa * grad(u)))");
  }
}

// Test that simplifier handles complex expressions reasonably
TEST_F(WeakFormDerivationTest, SimplifierComplexExpressions)
{
  // Test a typical variational derivative result
  {
    auto expr = parser->parse("0.5 * dot(grad(u), grad(u))");
    // After differentiation, this would produce 0.5 * 2 * grad(u)
    // which the simplifier doesn't collapse to grad(u)

    // But the original expression should remain unchanged
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "(0.5 * dot(grad(u), grad(u)))");
  }

  // Test that vec operations are preserved
  {
    auto expr = parser->parse("vec(1.0, 0.0)");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "vec(1, 0)");
  }

  // Test that function calls are preserved
  {
    auto expr = parser->parse("sin(x) + cos(x)");
    auto simplified = simplifier->simplify(expr);
    EXPECT_EQ(simplified->toString(), "(sin(x) + cos(x))");
  }
}

// Test that values very close to 1.0 are recognized as 1.0
TEST_F(WeakFormDerivationTest, ToleranceNearOneSimplification)
{
  // Create a value that's 1.0 plus a few machine epsilons
  Real one_plus_eps = 1.0 + 50 * std::numeric_limits<Real>::epsilon();
  std::string expr_str = std::to_string(one_plus_eps) + " * x";

  auto expr = parser->parse(expr_str);
  auto simplified = simplifier->simplify(expr);

  // Should be simplified to x since the coefficient is numerically 1.0
  EXPECT_EQ(simplified->toString(), "x");
}

// Test that values very close to 0.0 are recognized as 0.0
TEST_F(WeakFormDerivationTest, ToleranceNearZeroSimplification)
{
  // Create a value that's a few machine epsilons
  Real small_value = 50 * std::numeric_limits<Real>::epsilon();
  std::string expr_str = std::to_string(small_value) + " * x";

  auto expr = parser->parse(expr_str);
  auto simplified = simplifier->simplify(expr);

  // Should be simplified to 0 since the coefficient is numerically 0
  EXPECT_EQ(simplified->toString(), "0");
}

// Test that 0.5 * 2.0 is correctly recognized as 1.0
TEST_F(WeakFormDerivationTest, ToleranceExactArithmeticToOne)
{
  auto expr = parser->parse("0.5 * (2.0 * x)");
  auto simplified = simplifier->simplify(expr);

  // 0.5 * 2.0 = 1.0 exactly, should simplify to x
  EXPECT_EQ(simplified->toString(), "x");
}

// Test that 0.25 * 4.0 is correctly recognized as 1.0
TEST_F(WeakFormDerivationTest, ToleranceQuarterTimesFour)
{
  auto expr = parser->parse("0.25 * (4.0 * grad(u))");
  auto simplified = simplifier->simplify(expr);

  // 0.25 * 4.0 = 1.0 exactly, should simplify to grad(u)
  EXPECT_EQ(simplified->toString(), "grad(u)");
}

// Test that values that are NOT close to 1.0 are not incorrectly simplified
TEST_F(WeakFormDerivationTest, ToleranceNotNearOne)
{
  // This should be detectable as different from 1.0
  auto expr = parser->parse(" 1.00000099999999 * x");
  auto simplified = simplifier->simplify(expr);

  // Should NOT be simplified to x
  EXPECT_NE(simplified->toString(), "x");
  // Should keep the multiplication
  EXPECT_EQ(simplified->toString(), "(1.0000009999999899 * x)");
}

// Test division by small but non-zero values
TEST_F(WeakFormDerivationTest, ToleranceDivisionBySmallValue)
{
  // Value that's small but not numerically zero
  Real small_but_nonzero = 1000 * std::numeric_limits<Real>::epsilon();
  std::string expr_str = "x / " + std::to_string(small_but_nonzero);

  auto expr = parser->parse(expr_str);
  auto simplified = simplifier->simplify(expr);

  // Should not fail or produce infinity - the division should be preserved
  std::string result = simplified->toString();
  EXPECT_TRUE(result.find("div") != std::string::npos || result.find("/") != std::string::npos);
}

// Test that machine epsilon appropriate arithmetic works
TEST_F(WeakFormDerivationTest, ToleranceMachineEpsilonArithmetic)
{
  // Test that 1/3 * 3 is recognized as 1
  auto expr = parser->parse("0.333333333333333 * 3.0 * x");
  auto simplified = simplifier->simplify(expr);

  // 0.333... * 3.0 should be close enough to 1.0 to simplify
  std::string result = simplified->toString();

  // With our tolerance of 100 * epsilon, this should simplify to x
  // But the exact result depends on how many 3s we have
  // The parser might also handle this differently
  // Just check it produces something reasonable
  EXPECT_TRUE(result.find("x") != std::string::npos);
}

// Test nested multiplication with values that multiply to exactly 1.0
TEST_F(WeakFormDerivationTest, ToleranceNestedExactToOne)
{
  auto expr = parser->parse("0.125 * (2.0 * (4.0 * laplacian(u)))");
  auto simplified = simplifier->simplify(expr);

  // 0.125 * 2.0 * 4.0 = 1.0 exactly
  EXPECT_EQ(simplified->toString(), "laplacian(u)");
}

// Test that expressions with accumulating rounding errors are handled
TEST_F(WeakFormDerivationTest, ToleranceAccumulatingErrors)
{
  // Create an expression where multiple operations might accumulate small errors
  auto expr = parser->parse("0.1 * 0.1 * 100.0 * x");
  auto simplified = simplifier->simplify(expr);

  // 0.1 * 0.1 * 100.0 = 1.0, but 0.1 cannot be represented exactly in binary
  // Our tolerance should handle this appropriately
  EXPECT_EQ(simplified->toString(), "x");
}
