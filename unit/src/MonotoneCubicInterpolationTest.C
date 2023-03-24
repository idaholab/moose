//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MonotoneCubicInterpolation.h"
#include <cmath>

const double tol = 1e-8;

TEST(MonotoneCubicInterpolationTest, fitQuadraticFunction)
{
  std::vector<double> x(6);
  std::vector<double> y(6);

  x[0] = 0;
  y[0] = 5;
  x[1] = 2;
  y[1] = 1;
  x[2] = 4;
  y[2] = 5;
  x[3] = 6;
  y[3] = 17;
  x[4] = 8;
  y[4] = 37;
  x[5] = 10;
  y[5] = 65;

  MonotoneCubicInterpolation interp(x, y);

  EXPECT_NEAR(interp.sample(0.), 5., tol);
  EXPECT_NEAR(interp.sample(1.), 2., tol);
  EXPECT_NEAR(interp.sample(2.), 1., tol);
  EXPECT_NEAR(interp.sample(3.), 2., tol);
  EXPECT_NEAR(interp.sample(4.), 5., tol);
  EXPECT_NEAR(interp.sample(5.), 10., tol);
  EXPECT_NEAR(interp.sample(6.), 17., tol);
  EXPECT_NEAR(interp.sample(7.), 26., tol);
  EXPECT_NEAR(interp.sample(8.), 37., tol);
  EXPECT_NEAR(interp.sample(9.), 50., tol);
  EXPECT_NEAR(interp.sample(10.), 65., tol);

  EXPECT_NEAR(interp.sampleDerivative(0.), -4., tol);
  EXPECT_NEAR(interp.sampleDerivative(1.), -2., tol);
  EXPECT_NEAR(interp.sampleDerivative(2.), 0., tol);
  EXPECT_NEAR(interp.sampleDerivative(3.), 2., tol);
  EXPECT_NEAR(interp.sampleDerivative(4.), 4., tol);
  EXPECT_NEAR(interp.sampleDerivative(5.), 6., tol);
  EXPECT_NEAR(interp.sampleDerivative(6.), 8., tol);
  EXPECT_NEAR(interp.sampleDerivative(7.), 10., tol);
  EXPECT_NEAR(interp.sampleDerivative(8.), 12., tol);
  EXPECT_NEAR(interp.sampleDerivative(9.), 14., tol);
  EXPECT_NEAR(interp.sampleDerivative(10.), 16., tol);

  EXPECT_NEAR(interp.sample2ndDerivative(0.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(1.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(2.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(3.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(4.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(5.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(6.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(7.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(8.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(9.), 2., tol);
  EXPECT_NEAR(interp.sample2ndDerivative(10.), 2., tol);
}

TEST(MonotoneCubicInterpolationTest, fitAkimaDataSet)
{
  std::vector<double> x(11);
  std::vector<double> y(11);

  x[0] = 0;
  y[0] = 10;
  x[1] = 2;
  y[1] = 10;
  x[2] = 3;
  y[2] = 10;
  x[3] = 5;
  y[3] = 10;
  x[4] = 6;
  y[4] = 10;
  x[5] = 8;
  y[5] = 10;
  x[6] = 9;
  y[6] = 10.5;
  x[7] = 11;
  y[7] = 15;
  x[8] = 12;
  y[8] = 50;
  x[9] = 14;
  y[9] = 60;
  x[10] = 15;
  y[10] = 85;

  MonotoneCubicInterpolation interp(x, y);

  EXPECT_NEAR(interp.sample(0.), 10., tol);
  EXPECT_NEAR(interp.sample(2.), 10., tol);
  EXPECT_NEAR(interp.sample(3.), 10., tol);
  EXPECT_NEAR(interp.sample(5.), 10., tol);
  EXPECT_NEAR(interp.sample(6.), 10., tol);
  EXPECT_NEAR(interp.sample(8.), 10., tol);
  EXPECT_NEAR(interp.sample(9.), 10.5, tol);
  EXPECT_NEAR(interp.sample(11.), 15., tol);
  EXPECT_NEAR(interp.sample(12.), 50., tol);
  EXPECT_NEAR(interp.sample(14.), 60., tol);
  EXPECT_NEAR(interp.sample(15.), 85., tol);

  for (double z = 0; z <= 15.; z += .1)
    EXPECT_GE(interp.sampleDerivative(z), -tol);
}

TEST(MonotoneCubicInterpolationTest, monotoneIntervals)
{
  std::vector<double> x(11);
  std::vector<double> y(11);

  x[0] = 0;
  y[0] = 5;
  x[1] = 2;
  y[1] = 1;
  x[2] = 4;
  y[2] = 5;
  x[3] = 6;
  y[3] = 17;
  x[4] = 8;
  y[4] = 37;
  x[5] = 10;
  y[5] = 65;
  x[6] = 12;
  y[6] = 65;
  x[7] = 14;
  y[7] = 65;
  x[8] = 16;
  y[8] = 100;
  x[9] = 20;
  y[9] = 80;
  x[10] = 23;
  y[10] = 110;

  MonotoneCubicInterpolation interp(x, y);

  for (double z = 0; z < 2.; z += .1)
    EXPECT_LT(interp.sampleDerivative(z), 0);
  EXPECT_NEAR(interp.sampleDerivative(2.), 0, tol);
  for (double z = 2.1; z < 10; z += .1)
    EXPECT_GT(interp.sampleDerivative(z), 0);
  for (double z = 10.; z <= 14.; z += .1)
    EXPECT_NEAR(interp.sampleDerivative(z), 0, tol);
  for (double z = 14.1; z < 16.; z += .1)
    EXPECT_GT(interp.sampleDerivative(z), 0);
  EXPECT_NEAR(interp.sampleDerivative(16.), 0, tol);
  for (double z = 16.1; z < 20.; z += .1)
    EXPECT_LT(interp.sampleDerivative(z), 0);
  EXPECT_NEAR(interp.sampleDerivative(20), 0, tol);
  for (double z = 20.1; z <= 23; z += .1)
    EXPECT_GT(interp.sampleDerivative(z), 0);
}

TEST(MonotoneCubicInterpolationTest, getSampleSize)
{
  std::vector<double> x(6);
  std::vector<double> y(6);

  x[0] = 0.;
  y[0] = 0.;
  x[1] = 2.;
  y[1] = 4.;
  x[2] = 4.;
  y[2] = 16.;
  x[3] = 6.;
  y[3] = 36.;
  x[4] = 8.;
  y[4] = 64.;
  x[5] = 10.;
  y[5] = 100.;

  MonotoneCubicInterpolation interp(x, y);
  EXPECT_EQ(interp.getSampleSize(), x.size());
}

TEST(MonotoneCubicInterpolationTest, errors)
{
  try
  {
    std::vector<double> x(2);
    std::vector<double> y(3);
    MonotoneCubicInterpolation interp(x, y);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("MonotoneCubicInterpolation: x and y vectors are not the same length"),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    std::vector<double> x(3);
    std::vector<double> y(3);
    x[0] = 4;
    x[1] = 2;
    MonotoneCubicInterpolation interp(x, y);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("x-values are not strictly increasing"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    std::vector<double> x(2);
    std::vector<double> y(2);
    MonotoneCubicInterpolation interp(x, y);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("2 points is not enough data for a cubic interpolation"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}
