//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "NavierStokesMethods.h"
#include "NS.h"

namespace
{
// Residual of the equation that NS::findYplusThermal solves: the laminar sublayer
// temperature profile equals the logarithmic-layer temperature profile at y.
Real
thermalMatchResidual(const Real & y, const Real & Pr, const Real & Pr_t)
{
  const auto Pr_ratio = Pr / Pr_t;
  const auto jayatilleke_P =
      9.24 * (std::pow(Pr_ratio, 0.75) - 1.0) * (1.0 + 0.28 * std::exp(-0.007 * Pr_ratio));
  return Pr * y -
         Pr_t * (1.0 / NS::von_karman_constant * std::log(NS::E_turb_constant * y) + jayatilleke_P);
}
}

TEST(NavierStokesMethodsTest, findYplusThermalSatisfiesMatchingCondition)
{
  // A range of molecular-to-turbulent Prandtl number combinations spanning liquids
  // (Pr >> 1), gases (Pr ~ 1), and liquid metals (Pr << 1).
  const std::vector<std::pair<Real, Real>> cases = {
      {0.7, 0.9}, {1.0, 0.85}, {7.0, 0.9}, {50.0, 1.0}, {0.01, 0.9}, {0.1, 0.7}};

  for (const auto & [Pr, Pr_t] : cases)
  {
    const auto y_plus_thermal = NS::findYplusThermal<Real>(Pr, Pr_t);
    EXPECT_GT(y_plus_thermal, 0.0);
    EXPECT_NEAR(thermalMatchResidual(y_plus_thermal, Pr, Pr_t), 0.0, 1e-2 * Pr_t);
  }
}
