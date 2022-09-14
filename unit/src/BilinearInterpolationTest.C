//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "BilinearInterpolation.h"

const double tol = 1e-8;

Real
function(Real x1, Real x2)
{
   return x1 * x1 + 3.0 * x2 * x2;
}

TEST(BilinearInterpolationTest, sample)
{
  // Test BilinearInterpolation with a function y = x1 + 3 x2
  const unsigned int n = 9;
  std::vector<double> x1(n), x2(n);
  ColumnMajorMatrix y(n, n);

  for (unsigned int i = 0; i < n; ++i)
  {
    x1[i] = i;
    x2[i] = i;
  }

  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j < n; ++j)
      y(i, j) = function(x1[i], x2[j]);

  BilinearInterpolation interp(x1, x2, y);

  // Check sampled value and first and second derivatives
  Real p1 = 4.5, p2 = 5.5;
  EXPECT_NEAR(interp.sample(p1, p2), 4.5*4.5 + 3*5.5*5.5, tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 1), 9, tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 2), 33, tol);

  // Check that sampleValueAndDerivatives() returns the same results as above
  Real y2, dy2_dx1, dy2_dx2;
  interp.sampleValueAndDerivatives(p1, p2, y2, dy2_dx1, dy2_dx2);
  EXPECT_NEAR(y2, interp.sample(p1, p2), tol);
  EXPECT_NEAR(dy2_dx1, interp.sampleDerivative(p1, p2, 1), tol);
  EXPECT_NEAR(dy2_dx2, interp.sampleDerivative(p1, p2, 2), tol);

  // Check that the value and derivatives are correct at grid point
  p1 = 4, p2 = 8.25;
  EXPECT_NEAR(interp.sample(p1, p2), function(p1, p2), tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 1), 8.0, tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 2), 30.0, tol);
}
