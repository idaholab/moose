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

const double exact_tol = 1e-8;
const double apprx_tol = 1e-2;

Real
function(Real x1, Real x2)
{
  return x1 * x1 + 3.0 * x2 * x2;
}

void
function(Real x1, Real x2, Real & y, Real & dydx1, Real & dydx2)
{
  y = x1 * x1 + 3.0 * x2 * x2;
  dydx1 = 2 * x1;
  dydx2 = 6 * x2;
}

TEST(BilinearInterpolationTest, sample)
{
  // Test BilinearInterpolation with a function y = x1*x1 + 3 x2*x2
  const unsigned int n = 10000;
  const unsigned int m = 10000;
  const Real max_x1 = 10;
  const Real max_x2 = 10;
  std::vector<Real> x1(n), x2(m);
  ColumnMajorMatrix y(n, m);

  for (unsigned int i = 0; i < n; ++i)
    x1[i] = i / Real(n) * max_x1;
  for (unsigned int i = 0; i < m; ++i)
    x2[i] = i / Real(m) * max_x2;

  // Get function at all points on the grid
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j < m; ++j)
      y(i, j) = function(x1[i], x2[j]);

  BilinearInterpolation interp(x1, x2, y);

  // Within the domain, not at a grid point
  // Check sampled value and first and second derivatives
  Real p1 = 1.555555, p2 = 5.0111111;
  Real y0, dydx1, dydx2;
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);

  // Check that sampleValueAndDerivatives() returns the same results as above
  Real y2, dy2_dx1, dy2_dx2;
  interp.sampleValueAndDerivatives(p2, p1, y2, dy2_dx2, dy2_dx1);
  EXPECT_NEAR(y2, interp.sample(p2, p1), exact_tol);
  EXPECT_NEAR(dy2_dx1, interp.sampleDerivative(p2, p1, 2), exact_tol);
  EXPECT_NEAR(dy2_dx2, interp.sampleDerivative(p2, p1, 1), exact_tol);

  // Check that the value and derivatives are exact at a grid point inside the domain
  p1 = x1[4], p2 = x2[5];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, exact_tol);

  // Check that the value and derivatives are correct on grid lines inside the domain
  p1 = x1[4], p2 = x2[4] * 0.2 + 0.8 * x2[5];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[4] * 0.2 + x2[5] * 0.8, p2 = x2[4];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, exact_tol);

  // Check that the value and derivatives are correct at all four corners of the grid
  p1 = x1[0], p2 = x2[0];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[0], p2 = x2[m - 1];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[n - 1], p2 = x2[0];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[n - 1], p2 = x2[m - 1];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);

  // Check that the value and derivatives are correct at grid points on all bounds of the grid
  p1 = x1[0], p2 = x2[3];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, exact_tol);
  p1 = x1[n - 1], p2 = x2[3];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, exact_tol);
  p1 = x1[3], p2 = x2[0];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[3], p2 = x2[m - 1];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, exact_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);

  // Check that the value and derivatives are correct on the bounds of the grid, outside any grid
  // point
  p1 = x1[0], p2 = x2[3] * 0.6 + x2[4] * 0.4;
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[n - 1], p2 = x2[3] * 0.6 + x2[4] * 0.4;
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[3] * 0.6 + x1[4] * 0.4, p2 = x2[0];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);
  p1 = x1[3] * 0.6 + x1[4] * 0.4, p2 = x2[m - 1];
  function(p1, p2, y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(p2, p1), y0, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 2), dydx1, apprx_tol);
  EXPECT_NEAR(interp.sampleDerivative(p2, p1, 1), dydx2, apprx_tol);

  // Check that AD forwarding works, no need to redo all cases
  ADReal ad_p1 = 1.555555, ad_p2 = 5.0111111;
  function(ad_p1.value(), ad_p2.value(), y0, dydx1, dydx2);
  EXPECT_NEAR(interp.sample(ad_p2, ad_p1).value(), y0, apprx_tol);
}

TEST(BilinearInterpolationTest, unimplemented_errors)
{
  // Create the interpolation
  const unsigned int n = 100;
  const unsigned int m = 100;
  const Real max_x1 = 10;
  const Real max_x2 = 10;
  std::vector<Real> x1(n), x2(m);
  ColumnMajorMatrix y(n, m);

  for (unsigned int i = 0; i < n; ++i)
    x1[i] = i / Real(n) * max_x1;
  for (unsigned int i = 0; i < m; ++i)
    x2[i] = i / Real(m) * max_x2;

  // Get function at all points on the grid
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j < m; ++j)
      y(i, j) = function(x1[i], x2[j]);

  BilinearInterpolation interp(x1, x2, y);

  // These routines are currently unimplemented.
  // Second derivative is always 0 except where the discontinuities in the derivative happen
  try
  {
    interp.sample2ndDerivative(1, 2, 1);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("not implemented");
    ASSERT_TRUE(pos != std::string::npos);
  }

  // This AD routine is not implemented for now
  // Delete this if you implement them in the future
  try
  {
    ADReal y0, dydx1, dydx2;
    interp.sampleValueAndDerivatives(1, 2, y0, dydx1, dydx2);
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("not implemented");
    ASSERT_TRUE(pos != std::string::npos);
  }
}
