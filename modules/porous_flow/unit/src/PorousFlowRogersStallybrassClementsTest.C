//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PorousFlowRogersStallybrassClements.h"

const double eps = 1.0E-8;

TEST(PorousFlowRogersStallybrassClements, sat)
{
  EXPECT_NEAR(0.0, PorousFlowRogersStallybrassClements::effectiveSaturation(50, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(std::pow(2.0, -0.5),
              PorousFlowRogersStallybrassClements::effectiveSaturation(1.1, 1.1, 4.4),
              1.0E-5);
  EXPECT_NEAR(std::pow(1.0 + std::exp(1.0), -0.5),
              PorousFlowRogersStallybrassClements::effectiveSaturation(5.5, 1.1, 4.4),
              1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowRogersStallybrassClements::effectiveSaturation(-50, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowRogersStallybrassClements, dsat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowRogersStallybrassClements::dEffectiveSaturation(50, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::effectiveSaturation(1.1 + eps, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::effectiveSaturation(1.1, 1.1, 4.4)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowRogersStallybrassClements::dEffectiveSaturation(1.1, 1.1, 4.4), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::effectiveSaturation(5.5 + eps, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::effectiveSaturation(5.5, 1.1, 4.4)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowRogersStallybrassClements::dEffectiveSaturation(5.5, 1.1, 4.4), 1.0E-5);
  EXPECT_NEAR(
      0.0, PorousFlowRogersStallybrassClements::dEffectiveSaturation(-50, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowRogersStallybrassClements, d2sat)
{
  Real fd;
  EXPECT_NEAR(
      0.0, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(50, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::dEffectiveSaturation(1.1 + eps, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::dEffectiveSaturation(1.1, 1.1, 4.4)) /
       eps;
  EXPECT_NEAR(
      fd, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(1.1, 1.1, 4.4), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::dEffectiveSaturation(5.5 + eps, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::dEffectiveSaturation(5.5, 1.1, 4.4)) /
       eps;
  EXPECT_NEAR(
      fd, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(5.5, 1.1, 4.4), 1.0E-5);
  EXPECT_NEAR(
      0.0, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(-50, 0.7, 0.5), 1.0E-5);
}
