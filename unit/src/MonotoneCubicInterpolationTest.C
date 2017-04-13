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

#include "MonotoneCubicInterpolation.h"

#include <cmath>

const double tol = 1e-8;

TEST(MonotoneCubicInterpolationTest, fitQuadraticFunction)
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

  EXPECT_NEAR(interp.sample(0.), 0., tol);
  EXPECT_NEAR(interp.sample(1.), 1., tol);
  EXPECT_NEAR(interp.sample(2.), 4., tol);
  EXPECT_NEAR(interp.sample(3.), 9., tol);
  EXPECT_NEAR(interp.sample(4.), 16., tol);
  EXPECT_NEAR(interp.sample(5.), 25., tol);
  EXPECT_NEAR(interp.sample(6.), 36., tol);
  EXPECT_NEAR(interp.sample(7.), 49., tol);
  EXPECT_NEAR(interp.sample(8.), 64., tol);
  EXPECT_NEAR(interp.sample(9.), 81., tol);
  EXPECT_NEAR(interp.sample(10.), 100., tol);

  EXPECT_NEAR(interp.sampleDerivative(0.), 0., tol);
  EXPECT_NEAR(interp.sampleDerivative(1.), 2., tol);
  EXPECT_NEAR(interp.sampleDerivative(2.), 4., tol);
  EXPECT_NEAR(interp.sampleDerivative(3.), 6., tol);
  EXPECT_NEAR(interp.sampleDerivative(4.), 8., tol);
  EXPECT_NEAR(interp.sampleDerivative(5.), 10., tol);
  EXPECT_NEAR(interp.sampleDerivative(6.), 12., tol);
  EXPECT_NEAR(interp.sampleDerivative(7.), 14., tol);
  EXPECT_NEAR(interp.sampleDerivative(8.), 16., tol);
  EXPECT_NEAR(interp.sampleDerivative(9.), 18., tol);
  EXPECT_NEAR(interp.sampleDerivative(10.), 20., tol);

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
