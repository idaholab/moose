//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PorousFlowVanGenuchten.h"

const double eps = 1.0E-8;

TEST(PorousFlowVanGenuchten, sat)
{
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(
      0.486841442435055, PorousFlowVanGenuchten::effectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::effectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, dsat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::effectiveSaturation(-2.0 + eps, 0.7, 0.6) -
        PorousFlowVanGenuchten::effectiveSaturation(-2.0, 0.7, 0.6)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dEffectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::effectiveSaturation(-1.1 + eps, 0.9, 0.66) -
        PorousFlowVanGenuchten::effectiveSaturation(-1.1, 0.9, 0.66)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dEffectiveSaturation(-1.1, 0.9, 0.66), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2sat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dEffectiveSaturation(-2.0 + eps, 0.7, 0.6) -
        PorousFlowVanGenuchten::dEffectiveSaturation(-2.0, 0.7, 0.6)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2EffectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dEffectiveSaturation(-1.1 + eps, 2.3, 0.67) -
        PorousFlowVanGenuchten::dEffectiveSaturation(-1.1, 2.3, 0.67)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2EffectiveSaturation(-1.1, 2.3, 0.67), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, cap)
{
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::capillaryPressure(1.1, 1.0, 0.55, 1.0E30), 1.0E-5);
  EXPECT_NEAR(4.06172392297447,
              PorousFlowVanGenuchten::capillaryPressure(0.3, 0.625, 0.55, 1.0E30),
              1.0E-5);
  EXPECT_NEAR(2.9, PorousFlowVanGenuchten::capillaryPressure(0.001, 0.625, 0.55, 2.9), 1.0E-5);
  EXPECT_NEAR(1000.0, PorousFlowVanGenuchten::capillaryPressure(0.0, 0.625, 0.55, 1000.0), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, dcap)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dCapillaryPressure(0.0, 1.0, 0.55, 1.0E30), 1.0E-5);
  fd = (PorousFlowVanGenuchten::capillaryPressure(0.3 + eps, 0.625, 0.55, 1.0E30) -
        PorousFlowVanGenuchten::capillaryPressure(0.3, 0.625, 0.55, 1.0E30)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dCapillaryPressure(0.3, 0.625, 0.55, 1.0E30), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dCapillaryPressure(1.0, 1.0, 0.55, 1.0E30), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2cap)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2CapillaryPressure(0.0, 1.0, 0.55, 1.0E30), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dCapillaryPressure(0.3 + eps, 0.625, 0.55, 1.0E30) -
        PorousFlowVanGenuchten::dCapillaryPressure(0.3, 0.625, 0.55, 1.0E30)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2CapillaryPressure(0.3, 0.625, 0.55, 1.0E30), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2CapillaryPressure(1.0, 0.625, 0.55, 1.0E30), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, relperm)
{
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::relativePermeability(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::relativePermeability(-1.0, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0091160727, PorousFlowVanGenuchten::relativePermeability(0.3, 0.7), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, drelperm)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dRelativePermeability(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dRelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.3 + eps, 0.7) -
        PorousFlowVanGenuchten::relativePermeability(0.3, 0.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dRelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.8 + eps, 0.65) -
        PorousFlowVanGenuchten::relativePermeability(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dRelativePermeability(0.8, 0.65), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2relperm)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2RelativePermeability(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2RelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeability(0.3 + eps, 0.7) -
        PorousFlowVanGenuchten::dRelativePermeability(0.3, 0.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2RelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeability(0.8 + eps, 0.65) -
        PorousFlowVanGenuchten::dRelativePermeability(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2RelativePermeability(0.8, 0.65), 1.0E-5);
}
