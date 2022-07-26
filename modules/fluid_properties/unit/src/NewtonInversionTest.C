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
// WIP
/**
 * Test function z(x,y) for Newton's method 1D.
 * z(x) = x^2 - 3x + 4*y + 2 which has a root at 2 and 1 .
 * Test function f(x,y) for Newton's Method 2D
 * f(x,y) = y - 2x
 * Test function f(x,y) for Newton's Method 2D.
 * g(x,y) =
 * Note: the implementation of Brents bracketing method restricts the bracketing
 * interval to positive values
 */

void
function(Real x, Real y, Real & z, Real & dzdx, Real & dzdy)
{
   z = x * x - 3 * x + 4 * y + 2;
   dzdx = 2 * x - 3;
   dzdy = 4;
}

// void
// error(x)
// {
//   return 2;
// }
TEST(NewtonInversion, newtonSolve1D)
{
  Real x = 2;
  Real y = 3;
  Real soln = 12;
  Real guess = 11;
  auto func = [&](Real x, Real y, Real & z, Real & dzdx, Real & dzdy)
  {
    function(x, y, z, dzdx, dzdy);
  };

  // test that NewtonSolve gets correct roots
  // try
  // {
  // Real result = NewtonMethod::NewtonSolve(x, y, guess, 1e-8, func);
  // if (result != soln)
  //   FAIL() << "Newton Solve result does not match solution";
  // }
  // catch
  // {
  //
  // }
}

void
function1(Real x, Real y, Real & f, Real & dfdx, Real & dfdy)
{
  f = y - 2 * x ;
  dfdx = -2;
  dfdy = 1;
}
void
function2(Real x, Real y, Real & g, Real & dgdx, Real & dgdy)
{
  g = 3 * y - 4 * x;
  dgdx = 4;
  dgdy = 3;
}

TEST(NewtonInversion, NewtonSolve2D)
{
  Real x = 5;
  Real y = 8;
  Real guess1 = 2 ;
  Real guess2 = 10;
  Real res1;
  Real res2;
  Real f_soln = 1;
  Real g_soln = 11;
  bool converged;
  auto func1 = [&](Real x, Real y, Real & f, Real & dfdx, Real & dfdy)
  {
    function1(x, y, f, dfdx, dfdy);
  };
  auto func2 = [&](Real x, Real y, Real & g, Real & dgdx, Real & dgdy)
  {
    function1(x, y, g, dgdx, dgdy);
  };
  NewtonMethod::NewtonSolve2D(x, y, guess1, guess2, res1, res2, 1e-8, converged, func1, func2);

  // try
  // {
  //   Real result1 = res1;
  //   Real result2 = res2;
  //   if (result1 != f_soln || result2 != g_soln)
  //     FAIL() << "Newton Solve result does not match solution";
  // }
  // catch
  // {

  }
}
