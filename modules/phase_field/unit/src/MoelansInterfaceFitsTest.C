//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MoelansInterfaceFits.h"

using namespace MoelansInterfaceFits;

TEST(MoelansInterfaceFitsTest, moelans2022Table1)
{
  // gamma values from Table 1 of N. Moelans, Mater. Des. 217, 110592 (2022), which are given to
  // four decimals, with g^2 = sigma^2 / (kappa m)
  struct Sample
  {
    Real sigma, kappa, m, gamma;
  } samples[] = {{0.4, 0.36, 1.125, 5.1364},
                 {0.15, 0.36, 1.125, 0.6328},
                 {0.75, 0.6, 1.875, 13.7765},
                 {0.15, 0.6, 1.875, 0.5424}};

  for (const auto & s : samples)
  {
    const Real g2 = s.sigma * s.sigma / (s.kappa * s.m);
    EXPECT_NEAR(1.0 / inverseGamma(g2, Fit::MOELANS2022), s.gamma, 5e-5);
  }
}

TEST(MoelansInterfaceFitsTest, gamma1p5)
{
  // gamma = 1.5 is the one case with analytical values, g^2 = 2/9 and f0_interf = 1/8; the
  // tolerances reflect the accuracy of each fit there
  EXPECT_NEAR(inverseGamma(2.0 / 9.0, Fit::MOELANS2022), 1.0 / 1.5, 1e-3);
  EXPECT_NEAR(inverseGamma(2.0 / 9.0, Fit::MOELANS2009), 1.0 / 1.5, 1e-4);
  EXPECT_NEAR(f0Interf(1.0 / 1.5, Fit::MOELANS2022), 0.125, 1e-3);
  EXPECT_NEAR(f0Interf(1.0 / 1.5, Fit::MOELANS2009), 0.125, 1e-5);
}

TEST(MoelansInterfaceFitsTest, inRange)
{
  // the 2022 fit covers 0.098 <= g <= 0.766
  EXPECT_FALSE(inRange(0.097 * 0.097, Fit::MOELANS2022));
  EXPECT_TRUE(inRange(0.099 * 0.099, Fit::MOELANS2022));
  EXPECT_TRUE(inRange(0.765 * 0.765, Fit::MOELANS2022));
  EXPECT_FALSE(inRange(0.767 * 0.767, Fit::MOELANS2022));

  // the 2009 fit is not range checked
  EXPECT_TRUE(inRange(0.9, Fit::MOELANS2009));
}
