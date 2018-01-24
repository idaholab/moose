//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
