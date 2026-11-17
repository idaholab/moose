//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "WallFrictionModels.h"
#include "THMTestUtils.h"

TEST(WallFrictionModelsTest, darcy_friction_factor)
{
  ABS_TEST(WallFriction::DarcyFrictionFactor(5.), 20., 1e-13);
}

TEST(WallFrictionModelsTest, fanning_friction_factor_churchill)
{
  // Re < 10
  ABS_TEST(WallFriction::FanningFrictionFactorChurchill(1, 1e-5, 2e-2), 1.6, 1e-13);
  // Re > 10
  ABS_TEST(WallFriction::FanningFrictionFactorChurchill(100, 1e-5, 2e-2), 0.16, 1e-13);
}

TEST(WallFrictionModelsTest, fanning_friction_factor_cheng)
{
  // Laminar interior subchannel of a square array with P/D = 1.1, whose Darcy friction factor
  // constant from Table 5 of Cheng and Todreas (1986) is 35.55 + 263.7 (0.1) - 190.2 (0.1)^2 =
  // 60.018
  const Real Re = 2000;
  const Real f_F = WallFriction::FanningFrictionFactorCheng(Re, 35.55, 263.7, -190.2, 1.0, 1.1);
  REL_TEST(WallFriction::DarcyFrictionFactor(f_F), 60.018 / Re, REL_TOL_SAVED_VALUE);
}
