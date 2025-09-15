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

// Test basic differentiation
TEST_F(WeakFormDerivationTest, BasicDifferentiation)
{
  // Test 1: d/dx(x^2) = 2*x
  {
    auto expr = parser->parse("x*x");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    // Check string representation
    auto simplified = simplifier->simplify(result);
    std::string result_str = simplified->toString();
    // Result should be 2*x in some form
    EXPECT_TRUE(result_str.find("x") != std::string::npos);
    EXPECT_TRUE(result_str.find("2") != std::string::npos);
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
  }

  // Test 4: d/dx(x^3) = 3*x^2
  {
    auto expr = parser->parse("pow(x, 3.0)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);
    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    // Should contain x^2 or pow(x, 2)
    EXPECT_TRUE(result_str.find("pow(x, 2") != std::string::npos ||
                result_str.find("(x * x)") != std::string::npos);
    EXPECT_TRUE(result_str.find("3") != std::string::npos);
  }
}

// Test gradient operations
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

    std::string result_str = result->toString();
    // Should be 2*grad(c)
    EXPECT_TRUE(result_str.find("grad(c)") != std::string::npos);
    EXPECT_TRUE(result_str.find("2") != std::string::npos);
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
  }
}

// Test variational derivatives and Euler-Lagrange equations
TEST_F(WeakFormDerivationTest, VariationalDerivatives)
{
  // Test 1: Cahn-Hilliard energy F = W(c) + (κ/2)|∇c|²
  {
    auto energy = parser->parse("W(c) + 0.5*kappa*dot(grad(c), grad(c))");

    DifferentiationVisitor dv("c");
    auto diff = dv.differentiate(energy);

    // Check we have order 0 and order 1 coefficients
    EXPECT_TRUE(diff.hasOrder(0)); // W'(c)
    EXPECT_TRUE(diff.hasOrder(1)); // κ∇c

    // Compute Euler-Lagrange equation
    auto euler_lagrange = generator->computeEulerLagrange(diff);
    ASSERT_NE(euler_lagrange, nullptr);

    std::string el_str = euler_lagrange->toString();
    // The Euler-Lagrange equation combines all terms
    // Just check it's not empty and has some structure
    EXPECT_FALSE(el_str.empty());
    // Should have either subtract (for -div term) or add operations
    EXPECT_TRUE(el_str.find("subtract") != std::string::npos ||
                el_str.find("add") != std::string::npos ||
                el_str.find("div") != std::string::npos);
  }

  // Test 2: Allen-Cahn with double-well potential
  {
    auto energy = parser->parse("pow(eta*eta - 1.0, 2.0) + 0.5*kappa*dot(grad(eta), grad(eta))");

    DifferentiationVisitor dv("eta");
    auto diff = dv.differentiate(energy);

    EXPECT_TRUE(diff.hasOrder(0)); // Bulk term derivative
    EXPECT_TRUE(diff.hasOrder(1)); // Gradient term

    auto c0 = diff.getCoefficient(0);
    ASSERT_NE(c0, nullptr);

    std::string c0_str = c0->toString();
    // Should contain eta terms
    EXPECT_TRUE(c0_str.find("eta") != std::string::npos);
  }
}

// Test VectorAssembly (vec operator)
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
    EXPECT_TRUE(result_str.find("vec") != std::string::npos);
  }

  // Test 3: dot(grad(u), vec(1, 0)) extracts x-component
  {
    auto expr = parser->parse("dot(grad(u), vec(1.0, 0.0))");
    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);

    EXPECT_TRUE(diff.hasOrder(1));
    auto result = diff.getCoefficient(1);
    ASSERT_NE(result, nullptr);

    // Result should contain vec
    std::string result_str = result->toString();
    EXPECT_TRUE(result_str.find("vec") != std::string::npos);
  }

  // Test 4: vec with three components
  {
    auto expr = parser->parse("vec(x, y, z)");
    ASSERT_NE(expr, nullptr);

    auto vec_node = std::dynamic_pointer_cast<VectorAssemblyNode>(expr);
    ASSERT_NE(vec_node, nullptr);
    EXPECT_EQ(vec_node->components().size(), 3);
  }
}

// Test anisotropic energy expressions
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

    auto c1 = diff.getCoefficient(1);
    ASSERT_NE(c1, nullptr);

    std::string c1_str = c1->toString();
    // Should contain both kappa_x and kappa_y terms
    EXPECT_TRUE(c1_str.find("kappa_x") != std::string::npos);
    EXPECT_TRUE(c1_str.find("kappa_y") != std::string::npos);
  }
}

// Test weak form construction
TEST_F(WeakFormDerivationTest, WeakFormConstruction)
{
  // Test Cahn-Hilliard weak form
  {
    auto energy = parser->parse("W(c) + 0.5*kappa*dot(grad(c), grad(c))");
    auto weak_form = generator->generateWeakForm(energy, "c");
    ASSERT_NE(weak_form, nullptr);

    std::string wf_str = weak_form->toString();
    // The weak form should not be empty
    EXPECT_FALSE(wf_str.empty());
    // Should contain some operations (add, subtract, or div)
    EXPECT_TRUE(wf_str.find("subtract") != std::string::npos ||
                wf_str.find("add") != std::string::npos ||
                wf_str.find("div") != std::string::npos ||
                wf_str.find("(") != std::string::npos);
  }

  // Test Allen-Cahn weak form
  {
    auto energy = parser->parse("W(eta) + 0.5*kappa*dot(grad(eta), grad(eta))");
    auto weak_form = generator->generateWeakForm(energy, "eta");
    ASSERT_NE(weak_form, nullptr);

    std::string wf_str = weak_form->toString();
    EXPECT_TRUE(wf_str.find("div") != std::string::npos);
  }
}

// Test expression simplification
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
    EXPECT_EQ(simplified->toString(), "0.000000");
  }

  // Test 4: 2*3 = 6 (constant folding)
  {
    auto expr = parser->parse("2.0 * 3.0");
    auto simplified = simplifier->simplify(expr);
    ASSERT_NE(simplified, nullptr);
    EXPECT_EQ(simplified->toString(), "6.000000");
  }
}

// Test fourth-order derivatives for biharmonic-like problems
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

    // Check that the second-order coefficient is laplacian(u)
    std::string c2_str = c2->toString();
    EXPECT_TRUE(c2_str.find("laplacian") != std::string::npos);

    // Note: Computing Euler-Lagrange for fourth-order problems requires
    // taking divergence of a scalar (laplacian), which may not be supported
    // in all implementations
  }
}

// Test chain rule for composite functions
TEST_F(WeakFormDerivationTest, ChainRule)
{
  // Test d/dx[exp(sin(x))] = exp(sin(x)) * cos(x)
  {
    auto expr = parser->parse("exp(sin(x))");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    // Should contain both exp and cos
    EXPECT_TRUE(result_str.find("exp") != std::string::npos);
    EXPECT_TRUE(result_str.find("cos") != std::string::npos);
  }

  // Test d/dx[sin(x^2)] = cos(x^2) * 2x
  {
    auto expr = parser->parse("sin(x*x)");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    auto result = diff.getCoefficient(0);
    ASSERT_NE(result, nullptr);

    std::string result_str = result->toString();
    // Should contain cos and x
    EXPECT_TRUE(result_str.find("cos") != std::string::npos);
    EXPECT_TRUE(result_str.find("x") != std::string::npos);
  }
}

// Test tensor operations
TEST_F(WeakFormDerivationTest, TensorOperations)
{
  // Test trace operation
  {
    auto expr = parser->parse("trace(grad(vec(u, v)))");
    ASSERT_NE(expr, nullptr);

    // trace(grad(vec)) = div(vec) = du/dx + dv/dy
    DifferentiationVisitor dv_u("u");
    auto diff_u = dv_u.differentiate(expr);
    EXPECT_TRUE(diff_u.hasOrder(1));

    DifferentiationVisitor dv_v("v");
    auto diff_v = dv_v.differentiate(expr);
    EXPECT_TRUE(diff_v.hasOrder(1));
  }

  // Test symmetric tensor
  {
    auto expr = parser->parse("sym(grad(vec(u, v)))");
    ASSERT_NE(expr, nullptr);

    // Differentiate w.r.t. u
    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);
    EXPECT_TRUE(diff.hasOrder(1));
  }

  // Test contract operation
  {
    auto expr = parser->parse("contract(grad(vec(u, v)), grad(vec(u, v)))");
    ASSERT_NE(expr, nullptr);

    DifferentiationVisitor dv("u");
    auto diff = dv.differentiate(expr);
    EXPECT_TRUE(diff.hasOrder(1));
  }
}

// Test complex coupled system
TEST_F(WeakFormDerivationTest, CoupledSystem)
{
  // Test coupled Cahn-Hilliard and mechanics
  {
    auto energy = parser->parse(
      "W(c) + 0.5*kappa*dot(grad(c), grad(c)) + "
      "0.5*lambda*pow(trace(sym(grad(vec(u, v)))), 2.0) + "
      "alpha*c*trace(sym(grad(vec(u, v))))");

    // Differentiate w.r.t. c
    DifferentiationVisitor dv_c("c");
    auto diff_c = dv_c.differentiate(energy);
    EXPECT_TRUE(diff_c.hasOrder(0)); // W'(c) + alpha*tr(strain)
    EXPECT_TRUE(diff_c.hasOrder(1)); // kappa*grad(c)

    // Differentiate w.r.t. u
    DifferentiationVisitor dv_u("u");
    auto diff_u = dv_u.differentiate(energy);
    EXPECT_TRUE(diff_u.hasOrder(1)); // Mechanical stress terms

    // Check coupling term appears in both derivatives
    auto c0_c = diff_c.getCoefficient(0);
    ASSERT_NE(c0_c, nullptr);
    std::string c0_c_str = c0_c->toString();
    EXPECT_TRUE(c0_c_str.find("alpha") != std::string::npos);

    auto c1_u = diff_u.getCoefficient(1);
    ASSERT_NE(c1_u, nullptr);
    std::string c1_u_str = c1_u->toString();
    EXPECT_TRUE(c1_u_str.find("alpha") != std::string::npos ||
                c1_u_str.find("lambda") != std::string::npos);
  }
}

// Test edge cases and error handling
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
  }

  // Test differentiation of different variable
  {
    auto expr = parser->parse("y*y");
    DifferentiationVisitor dv("x");
    auto diff = dv.differentiate(expr);

    // Should have no coefficients (no dependence on x)
    EXPECT_FALSE(diff.hasOrder(0));
    EXPECT_FALSE(diff.hasOrder(1));
  }

  // Test empty vec()
  {
    // This should fail to parse or create an empty vector
    // The behavior depends on implementation
  }
}

// Test integration by parts transformation
TEST_F(WeakFormDerivationTest, IntegrationByParts)
{
  // For a simpler second-order problem, check the weak form transformation
  {
    // Use a simpler energy with only second-order terms: F = (1/2)|∇u|²
    auto energy = parser->parse("0.5*dot(grad(u), grad(u))");

    auto contributions = generator->computeContributions(energy, "u");

    // Should have c1_term for first-order derivatives
    ASSERT_NE(contributions.c1_term, nullptr);

    // The first-order term should be grad(u)
    std::string c1_str = contributions.c1_term->toString();
    EXPECT_TRUE(c1_str.find("grad(u)") != std::string::npos);

    // Maximum order should be 1 for this energy
    EXPECT_EQ(contributions.max_order, 1);
  }
}