//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMFrictionMATRA.h"

#include "gtest/gtest.h"

#include <cmath>

TEST(FrictionTests, matraFrictionFactorIsContinuousAtTransition)
{
  const Real Re = 5000.0;
  const Real eps = 1e-6;

  const auto below = SCMFrictionMATRA::computeQuadLatticeFrictionFactor(Re - eps);
  const auto at = SCMFrictionMATRA::computeQuadLatticeFrictionFactor(Re);

  EXPECT_NEAR(below, at, 1e-10);
}

TEST(FrictionTests, matraFrictionFactorPreservesLaminarAndTurbulentBranches)
{
  EXPECT_NEAR(SCMFrictionMATRA::computeQuadLatticeFrictionFactor(1000.0), 64.0 / 1000.0, 1e-14);
  EXPECT_NEAR(SCMFrictionMATRA::computeQuadLatticeFrictionFactor(10000.0),
              0.316 * std::pow(10000.0, -0.25),
              1e-14);
  EXPECT_NEAR(SCMFrictionMATRA::computeQuadLatticeFrictionFactor(100000.0),
              0.184 * std::pow(100000.0, -0.20),
              1e-14);
}
