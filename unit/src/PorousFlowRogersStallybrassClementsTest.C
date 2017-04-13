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
