//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistryActivityCalculators.h"

const Real A = 0.4913;
const Real B = 0.3247;
const Real Bdot = 0.0174;
const Real a = 0.1224;
const Real b = -0.004679;
const Real c = 0.0004114;
const Real d = 0.0;
const Real atilde = 1.5551;
const Real btilde = 0.036478;
const Real ctilde = 0.0064366;
const Real dtilde = -0.0007132;

TEST(GeochemistryActivityCalculatorsTest, log10ActCoeffDHBdot)
{
  const Real charge = 2.0;
  const Real ionic_rad = 1.5;
  const Real ionic_str = 0.2;
  const Real log10gold = -0.718192735532499;
  ASSERT_NEAR(GeochemistryActivityCalculators::log10ActCoeffDHBdot(
                  charge, ionic_rad, std::sqrt(ionic_str), A, B, Bdot),
              log10gold,
              1E-9);
  ASSERT_EQ(
      GeochemistryActivityCalculators::log10ActCoeffDHBdot(charge, ionic_rad, 0.0, A, B, Bdot),
      0.0);
  ASSERT_EQ(
      GeochemistryActivityCalculators::log10ActCoeffDHBdot(charge, ionic_rad, -1.0, A, B, Bdot),
      0.0);
}

TEST(GeochemistryActivityCalculatorsTest, log10ActCoeffDavies)
{
  const Real charge = 2.0;
  const Real ionic_str = 0.2;
  const Real log10gold = -0.489368197345647;
  ASSERT_NEAR(GeochemistryActivityCalculators::log10ActCoeffDavies(charge, std::sqrt(ionic_str), A),
              log10gold,
              1E-9);
  ASSERT_EQ(GeochemistryActivityCalculators::log10ActCoeffDavies(charge, 0.0, A), 0.0);
  ASSERT_EQ(GeochemistryActivityCalculators::log10ActCoeffDavies(charge, -1.0, A), 0.0);
}

TEST(GeochemistryActivityCalculatorsTest, log10ActCoeffDHBdotAlternative)
{
  const Real ionic_str = 0.2;
  const Real log10gold = 0.00348;
  ASSERT_NEAR(GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(ionic_str, Bdot),
              log10gold,
              1E-9);
  ASSERT_EQ(GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(0.0, Bdot), 0.0);
  ASSERT_EQ(GeochemistryActivityCalculators::log10ActCoeffDHBdotAlternative(-1.0, Bdot), 0.0);
}

TEST(GeochemistryActivityCalculatorsTest, log10ActCoeffDHBdotNeutral)
{
  const Real ionic_str = 0.2;
  const Real log10gold = 0.0242961312;
  ASSERT_NEAR(GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(ionic_str, a, b, c, d),
              log10gold,
              1E-9);
  ASSERT_EQ(GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(0.0, a, b, c, d), 0.0);
  ASSERT_EQ(GeochemistryActivityCalculators::log10ActCoeffDHBdotNeutral(-1.0, a, b, c, d), 0.0);
}

TEST(GeochemistryActivityCalculatorsTest, lnActivityDHBdotWater)
{
  const Real ionic_str = 0.2;
  const Real loggold = -0.0066955098152888023;
  ASSERT_NEAR(GeochemistryActivityCalculators::lnActivityDHBdotWater(
                  ionic_str, A, atilde, btilde, ctilde, dtilde),
              loggold,
              1E-9);
  ASSERT_EQ(GeochemistryActivityCalculators::lnActivityDHBdotWater(
                0.0, A, atilde, btilde, ctilde, dtilde),
            0.0);
  ASSERT_EQ(GeochemistryActivityCalculators::lnActivityDHBdotWater(
                -1.0, A, atilde, btilde, ctilde, dtilde),
            0.0);
}
