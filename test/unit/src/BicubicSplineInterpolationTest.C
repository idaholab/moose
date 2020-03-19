//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "BicubicSplineInterpolation.h"

const double tol = 1e-8;

TEST(BicubicSplineInterpolationTest, sample)
{
  // Test BicubicSplineInterpolation with a function y = x1^2 + x2^3
  const unsigned int n = 5;
  std::vector<double> x1(n), x2(n), yx11(n), yx1n(n), yx21(n), yx2n(n);
  std::vector<std::vector<double>> y(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    x1[i] = i;
    x2[i] = i;
    yx1n[i] = 8.0;
    yx2n[i] = 48.0;
    y[i].resize(n);
  }

  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j < n; ++j)
      y[i][j] = x1[i] * x1[i] + x2[j] * x2[j] * x2[j];

  BicubicSplineInterpolation interp(x1, x2, y, yx11, yx1n, yx21, yx2n);

  // Check sampled value and first and second derivatives
  EXPECT_NEAR(interp.sample(2.0, 3.0), 31.0, tol);
  EXPECT_NEAR(interp.sampleDerivative(2.0, 3.0, 1, yx11[0], yx1n[0]), 4.0, tol);
  EXPECT_NEAR(interp.sampleDerivative(2.0, 3.0, 2, yx21[0], yx2n[0]), 27.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(2.0, 3.0, 1, yx11[0], yx1n[0]), 2.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(2.0, 3.0, 2, yx21[0], yx2n[0]), 18.0, tol);

  // Check that sampleValueAndDerivatives() returns the same results as above
  double y2, dy2_dx1, dy2_dx2;
  interp.sampleValueAndDerivatives(
      2.0, 3.0, y2, dy2_dx1, dy2_dx2, yx11[0], yx1n[0], yx21[0], yx2n[0]);
  EXPECT_NEAR(y2, 31.0, tol);
  EXPECT_NEAR(dy2_dx1, 4.0, tol);
  EXPECT_NEAR(dy2_dx2, 27.0, tol);
}
