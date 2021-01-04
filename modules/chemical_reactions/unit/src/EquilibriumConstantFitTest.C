//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "EquilibriumConstantFit.h"

#include <cmath>

const double tol = 1.0e-6;
const std::vector<double> x = {273.15, 298.15, 333.15, 373.15, 423.15, 473.15, 523.15, 573.15};
const std::vector<double> y = {
    14.9398, 13.9951, 13.0272, 12.2551, 11.6308, 11.2836, 11.1675, 11.3002};

TEST(EquilibriumConstantFitTest, constructor)
{
  EquilibriumConstantFit logk(x, y);
  EXPECT_EQ(logk.getSampleSize(), x.size());
}

TEST(EquilibriumConstantFitTest, sample)
{
  EquilibriumConstantFit logk(x, y);
  logk.generate();

  // Compare with values calculated using scipy.optimize.curve_fit
  EXPECT_NEAR(logk.sample(x[1]), 13.991103115875013, tol);
  EXPECT_NEAR(logk.sample(x[2]), 13.028890219017683, tol);
  EXPECT_NEAR(logk.sample(x[3]), 12.259284844918287, tol);
  EXPECT_NEAR(logk.sample(x[4]), 11.627599702911748, tol);
  EXPECT_NEAR(logk.sample(x[5]), 11.278660590504469, tol);
}
