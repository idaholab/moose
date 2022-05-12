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
#include "DualReal.h"
#include "ChainedReal.h"

#include "DualRealOps.h"

#include <cmath>

TEST(LinearInterpolationTest, getSampleSize)
{
  std::vector<Real> x = {1, 2, 3, 5};
  std::vector<Real> y = {0, 5, 6, 8};
  LinearInterpolation interp(x, y);
  ASSERT_EQ(interp.getSampleSize(), x.size());
}

TEST(LinearInterpolationTest, sample)
{
  std::vector<Real> x = {1, 2, 3, 5};
  std::vector<Real> y = {0, 5, 6, 8};
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

TEST(LinearInterpolationTest, automatic_differentiation_sample)
{
  std::vector<Real> x = {1, 2};
  std::vector<Real> y = {0, 5};
  ADLinearInterpolation interp(x, y);

  ADReal xx = 1.5;
  Moose::derivInsert(xx.derivatives(), 0, 1);
  auto yy = interp.sample(xx);

  EXPECT_DOUBLE_EQ(yy.value(), 2.5);
  EXPECT_DOUBLE_EQ(yy.derivatives()[0], 5.0);
}

TEST(LinearInterpolationTest, chained_real_sample)
{
  std::vector<Real> x = {1, 2};
  std::vector<Real> y = {0, 5};
  ADLinearInterpolation interp(x, y);

  ChainedReal xx(1.5, 1);
  auto yy = interp.sample(xx);

  EXPECT_DOUBLE_EQ(yy.value(), 2.5);
  EXPECT_DOUBLE_EQ(yy.derivatives(), 5.0);
}
