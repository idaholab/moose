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

#include "LinearInterpolation.h"

#include <cmath>

TEST(LinearInterpolationTest, getSampleSize)
{
  std::vector<double> x = {1, 2, 3, 5};
  std::vector<double> y = {0, 5, 6, 8};
  LinearInterpolation interp(x, y);
  ASSERT_EQ(interp.getSampleSize(), x.size());
}

TEST(LinearInterpolationTest, sample)
{
  std::vector<double> x = {1, 2, 3, 5};
  std::vector<double> y = {0, 5, 6, 8};
  LinearInterpolation interp(x, y);

  EXPECT_DOUBLE_EQ(interp.sample(0.), 0.);
  EXPECT_DOUBLE_EQ(interp.sample(1.), 0.);
  EXPECT_DOUBLE_EQ(interp.sample(2.), 5.);
  EXPECT_DOUBLE_EQ(interp.sample(3.), 6.);
  EXPECT_DOUBLE_EQ(interp.sample(4.), 7.);
  EXPECT_DOUBLE_EQ(interp.sample(5.), 8.);
  EXPECT_DOUBLE_EQ(interp.sample(6.), 8.);

  EXPECT_DOUBLE_EQ(interp.sample(1.5), 2.5);

  EXPECT_DOUBLE_EQ(interp.sampleDerivative(0.), 0.);
  EXPECT_DOUBLE_EQ(interp.sampleDerivative(1.1), 5.);
  EXPECT_DOUBLE_EQ(interp.sampleDerivative(2.), 1.);
  EXPECT_DOUBLE_EQ(interp.sampleDerivative(2.1), 1.);
}

