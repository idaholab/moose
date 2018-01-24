//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
