//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PorousFlowBrooksCorey.h"

const Real eps = 1.0e-8;
const Real tol = 1.0e-6;

TEST(PorousFlowBrooksCoreyTest, sat)
{
  EXPECT_NEAR(1.0, PorousFlowBrooksCorey::effectiveSaturation(0.99, 1.0, 2.0), tol);
  EXPECT_NEAR(1.0, PorousFlowBrooksCorey::effectiveSaturation(1.0, 1.0, 2.0), tol);

  const Real seff = std::pow(2.5 / 1.2, -2.0);
  EXPECT_NEAR(seff, PorousFlowBrooksCorey::effectiveSaturation(2.5, 1.2, 2.0), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::effectiveSaturation(1.0e9, 1.2, 2.0), tol);
}

TEST(PorousFlowBrooksCoreyTest, dsat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dEffectiveSaturation(0.99, 1.0, 2.0), tol);

  fd = (PorousFlowBrooksCorey::effectiveSaturation(2.5 + eps, 1.2, 2.0) -
        PorousFlowBrooksCorey::effectiveSaturation(2.5, 1.2, 2.0)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowBrooksCorey::dEffectiveSaturation(2.5, 1.2, 2.0), tol);

  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dEffectiveSaturation(1.0e9, 1.2, 2.0), tol);
}

TEST(PorousFlowBrooksCoreyTest, d2sat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::d2EffectiveSaturation(0.99, 1.0, 2.0), tol);

  fd = (PorousFlowBrooksCorey::dEffectiveSaturation(2.5 + eps, 1.2, 2.0) -
        PorousFlowBrooksCorey::dEffectiveSaturation(2.5, 1.2, 2.0)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowBrooksCorey::d2EffectiveSaturation(2.5, 1.2, 2.0), tol);

  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::d2EffectiveSaturation(1.0e9, 1.2, 2.0), tol);
}

TEST(PorousFlowBrooksCoreyTest, cap)
{
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::capillaryPressure(1.3, 1.2, 2.0, 1.0e9), tol);
  const Real pc = 1.2 * std::pow(0.3, -1.0 / 2.0);
  EXPECT_NEAR(pc, PorousFlowBrooksCorey::capillaryPressure(0.3, 1.2, 2.0, 1.0e9), tol);
  EXPECT_NEAR(1000.0, PorousFlowBrooksCorey::capillaryPressure(0.0, 1.2, 2.0, 1000.0), tol);
}

TEST(PorousFlowBrooksCoreyTest, dcap)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dCapillaryPressure(1.3, 1.2, 2.0, 1.0e9), tol);
  fd = (PorousFlowBrooksCorey::capillaryPressure(0.3 + eps, 1.2, 2.0, 1.0e9) -
        PorousFlowBrooksCorey::capillaryPressure(0.3, 1.2, 2.0, 1.0e9)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowBrooksCorey::dCapillaryPressure(0.3, 1.2, 2.0, 1.0e9), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dCapillaryPressure(1.0, 1.2, 2.0, 1.0e9), tol);
}

TEST(PorousFlowBrooksCoreyTest, d2cap)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::d2CapillaryPressure(1.3, 1.2, 2.0, 1.0e9), tol);
  fd = (PorousFlowBrooksCorey::dCapillaryPressure(0.3 + eps, 1.2, 2.0, 1.0e9) -
        PorousFlowBrooksCorey::dCapillaryPressure(0.3, 1.2, 2.0, 1.0e9)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowBrooksCorey::d2CapillaryPressure(0.3, 1.2, 2.0, 1.0e9), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::d2CapillaryPressure(1.0, 1.2, 2.0, 1.0e9), tol);
}

TEST(PorousFlowBrooksCoreyTest, relpermw)
{
  EXPECT_NEAR(1.0, PorousFlowBrooksCorey::relativePermeabilityW(1.1, 2.5), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::relativePermeabilityW(-1.0, 2.5), tol);
  const Real relperm = std::pow(0.3, (2.0 + 3.0 * 2.5) / 2.5);
  EXPECT_NEAR(relperm, PorousFlowBrooksCorey::relativePermeabilityW(0.3, 2.5), tol);
}

TEST(PorousFlowBrooksCoreyTest, drelpermw)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dRelativePermeabilityW(1.1, 2.5), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dRelativePermeabilityW(-1.0, 2.5), tol);
  fd = (PorousFlowBrooksCorey::relativePermeabilityW(0.3 + eps, 2.5) -
        PorousFlowBrooksCorey::relativePermeabilityW(0.3, 2.5)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowBrooksCorey::dRelativePermeabilityW(0.3, 2.5), tol);
}

TEST(PorousFlowBrooksCoreyTest, relpermnw)
{
  EXPECT_NEAR(1.0, PorousFlowBrooksCorey::relativePermeabilityNW(1.1, 2.5), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::relativePermeabilityNW(-1.0, 2.5), tol);
  const Real relperm = 0.3 * 0.3 * (1.0 - std::pow(1.0 - 0.3, (2.0 + 2.5) / 2.5));
  EXPECT_NEAR(relperm, PorousFlowBrooksCorey::relativePermeabilityNW(0.3, 2.5), tol);
}

TEST(PorousFlowBrooksCoreyTest, drelpermnw)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dRelativePermeabilityNW(1.1, 2.5), tol);
  EXPECT_NEAR(0.0, PorousFlowBrooksCorey::dRelativePermeabilityNW(-1.0, 2.5), tol);
  fd = (PorousFlowBrooksCorey::relativePermeabilityNW(0.3 + eps, 2.5) -
        PorousFlowBrooksCorey::relativePermeabilityNW(0.3, 2.5)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowBrooksCorey::dRelativePermeabilityNW(0.3, 2.5), tol);
}
