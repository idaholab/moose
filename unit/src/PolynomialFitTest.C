/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "gtest/gtest.h"

#include "PolynomialFit.h"

#include <cmath>

const double tol = 1e-5;

TEST(PolynomialFitTest, constructor)
{
  std::vector<double> x = {-1, 0, 1};
  std::vector<double> y = {1, 0, 1};
  PolynomialFit poly(x, y, 2);
  EXPECT_EQ(poly.getSampleSize(), x.size());
}

TEST(PolynomialFitTest, sample)
{
  std::vector<double> x = {-1, 0, 1};
  std::vector<double> y = {1, 0, 1};
  PolynomialFit poly(x, y, 2);
  poly.generate();

  EXPECT_NEAR(poly.sample(-2.), 4, tol);
  EXPECT_NEAR(poly.sample(-1.), 1, tol);
  EXPECT_NEAR(poly.sample(0.), 0, tol);
  EXPECT_NEAR(poly.sample(1.), 1, tol);
  EXPECT_NEAR(poly.sample(2.), 4, tol);
}

TEST(PolynomialFitTest, getSampleSize)
{
  std::vector<double> x = {-1, 0, 1};
  std::vector<double> y = {1, 0, 1};
  PolynomialFit poly(x, y, 2);
  EXPECT_EQ(poly.getSampleSize(), x.size());
}
