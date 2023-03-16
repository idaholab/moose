//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "NewtonInversion.h"

void
function_f1(Real x1, Real x2, Real & z, Real & dzdx1, Real & dzdx2)
{
  z = x2 * x2 - 3 * x2 + 4 * x1 + 2;
  dzdx1 = 4;
  dzdx2 = 2 * x2 - 3;
}

void
function_f2(Real x1, Real x2, Real & z, Real & dzdx1, Real & dzdx2)
{
  z = exp(x1 * x2) + x2 * log(x2);
  dzdx1 = x2 * exp(x1 * x2);
  dzdx2 = x1 * exp(x1 * x2) + log(x2) + 1;
}

/**
 * Tests the implementation of Newton's method to find the roots of:
 * a polynomial of x and y, with x constant
 * f1(x, y) = y^2 - 3*y + 4*x + 2
 * a non-linear function of x and y, with x constant
 * f2(x, y) = exp(x * y) + y * log(y)
 */
TEST(NewtonInversion, NewtonSolve)
{
  // Test 2nd degree polynomial with two roots (f1)
  // x is kept constant when calling f_1(x, y)
  Real x = 0;
  // we seek y such that f(x1, x2) = z
  Real z = 0;
  Real initial_guess = 11;
  auto func = [&](Real x1, Real x2, Real & z, Real & dzdx1, Real & dzdx2)
  { function_f1(x1, x2, z, dzdx1, dzdx2); };

  // Solve z = f(x, y) with x constant
  Real y = FluidPropertiesUtils::NewtonSolve(x, z, initial_guess, 1e-8, func, "unit").first;

  // Check solution found
  Real tol = 1e-7;
  Real soln = 2;
  EXPECT_NEAR(y, soln, tol);

  // Test non linear function (f2)
  auto func2 = [&](Real x1, Real x2, Real & z, Real & dzdx1, Real & dzdx2)
  { function_f2(x1, x2, z, dzdx1, dzdx2); };

  x = 1;
  z = 0.8749124087762432;
  soln = 0.1;
  initial_guess = 0.1;
  y = FluidPropertiesUtils::NewtonSolve(x, z, initial_guess, 1e-8, func2, "unit").first;
  EXPECT_NEAR(y, soln, tol);
}

void
function_g1(Real x1, Real x2, Real & g, Real & dgdx1, Real & dgdx2)
{
  g = x2 - 2 * x1;
  dgdx1 = -2;
  dgdx2 = 1;
}

void
function_g2(Real x1, Real x2, Real & g, Real & dgdx1, Real & dgdx2)
{
  g = x1 * x1 + x2 * x2 - 4 * x1 * x2 + 2;
  dgdx1 = 2 * x1 - 4 * x2;
  dgdx2 = 2 * x2 - 4 * x1;
}

void
function_g3(Real x1, Real x2, Real & g, Real & dgdx1, Real & dgdx2)
{
  g = exp(x2) + x1 * log(x1);
  dgdx1 = 1 + log(x1);
  dgdx2 = exp(x2);
}

/**
 * Tests the implementation of Newton's method to find the roots of:
 * 2D inversion problems, where we seek x1,x2 such that g_i(x1,x2)=y1, g_j(x1,x2)=y2
 * with the g_j functions chosen among:
 * a 2D bilinear function
 * g1(x,y) = y - 2x
 * a 2D quadratic function
 * g2(x,y) = y^2 + x^2 - 4*x*y + 2
 * a 2D nonlinear function
 * g3(x,y) = exp(y) + x*log(x)
 */
TEST(NewtonInversion, NewtonSolve2D)
{
  // Initial guess
  Real guess1 = 2;
  Real guess2 = 10;

  // Target values
  Real y1 = -3;
  Real y2 = -37;

  // Roots of the problem obtained by Newton's method
  Real return_x1;
  Real return_x2;

  // Known solution for the first problem
  Real x1_soln = -4;
  Real x2_soln = -11;

  auto func1 = [&](Real x, Real y, Real & f, Real & dfdx, Real & dfdy)
  { function_g1(x, y, f, dfdx, dfdy); };
  auto func2 = [&](Real x, Real y, Real & g, Real & dgdx, Real & dgdy)
  { function_g2(x, y, g, dgdx, dgdy); };
  auto func3 = [&](Real x, Real y, Real & g, Real & dgdx, Real & dgdy)
  { function_g3(x, y, g, dgdx, dgdy); };
  FluidPropertiesUtils::NewtonSolve2D(
      y1, y2, guess1, guess2, return_x1, return_x2, 1e-8, 1e-8, func1, func2);

  // Check values
  Real tol = 1e-6;
  EXPECT_NEAR(return_x1, x1_soln, tol);
  EXPECT_NEAR(return_x2, x2_soln, tol);

  // Try other combinations of g functions
  y1 = 0.1;
  y2 = 1.1196002982765987;
  FluidPropertiesUtils::NewtonSolve2D(
      y1, y2, guess1, guess2, return_x1, return_x2, 1e-8, 1e-8, func1, func3);
  x1_soln = 0.1;
  x2_soln = 0.3;
  EXPECT_NEAR(return_x1, x1_soln, tol);
  EXPECT_NEAR(return_x2, x2_soln, tol);

  y1 = 1.98;
  y2 = 1.1196002982765987;
  FluidPropertiesUtils::NewtonSolve2D(
      y1, y2, guess1, guess2, return_x1, return_x2, 1e-8, 1e-8, func2, func3);
  x1_soln = 0.1;
  x2_soln = 0.3;
  EXPECT_NEAR(return_x1, x1_soln, tol);
  EXPECT_NEAR(return_x2, x2_soln, tol);

  // If there is no solution it should not converge
  y1 = -2000;
  y2 = -2000; // no solution
  try
  {
    FluidPropertiesUtils::NewtonSolve2D(
        y1, y2, guess1, guess2, return_x1, return_x2, 1e-8, 1e-8, func1, func3);
    FAIL();
  }
  catch (MooseException &)
  {
  }
}
