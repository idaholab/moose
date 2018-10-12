//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalGraphitePropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(ThermalGraphitePropertiesTest, name)
{
  EXPECT_EQ(_sp->solidName(), "thermal_graphite");
}

TEST_F(ThermalGraphitePropertiesTest, molarMass)
{
  EXPECT_EQ(_sp->molarMass(), 12.0107e-3);
}

TEST_F(ThermalGraphitePropertiesTest, valuesAbove100)
{
  Real T = 800.0 + 273.15;

  REL_TEST(_sp->cp_from_T(T), 1796.92593135747, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->k_from_T(T), 72.23751080759918, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->rho_from_T(T), 1594.9290549301618, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->beta_from_T(T), 4.063257267498634e-06, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->emissivity_from_T(T), 0.787911115, REL_TOL_SAVED_VALUE);
}

TEST_F(ThermalGraphitePropertiesTest, valuesBelow100)
{
  Real T = 95.0 + 273.15;

  REL_TEST(_sp2->cp_from_T(T), 905.8291394590268, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp2->k_from_T(T), 127.10153195286183, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp2->rho_from_T(T), 1599.649, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp2->beta_from_T(T), 2.925e-6, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp2->emissivity_from_T(T), 0.4988047, REL_TOL_SAVED_VALUE);
}

TEST_F(ThermalGraphitePropertiesTest, valuesConstantDensityEmissivity)
{
  Real T = 95.0 + 273.15;
  REL_TEST(_sp3->rho_from_T(T), 1884.586478125, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp3->emissivity_from_T(T), 0.8, REL_TOL_SAVED_VALUE);
}

TEST_F(ThermalGraphitePropertiesTest, derivativesAbove100)
{
  Real T = 800.0 + 273.15;
  Real dT = 1.0e-6 * T;

  Real drho_dT_fd = (_sp->rho_from_T(T + dT) - _sp->rho_from_T(T - dT)) / (2 * dT);
  Real dcp_dT_fd = (_sp->cp_from_T(T + dT) - _sp->cp_from_T(T - dT)) / (2 * dT);
  Real dk_dT_fd = (_sp->k_from_T(T + dT) - _sp->k_from_T(T - dT)) / (2 * dT);

  Real rho, drho_dT;
  Real cp, dcp_dT;
  Real k, dk_dT;

  _sp->rho_from_T(T, rho, drho_dT);
  _sp->cp_from_T(T, cp, dcp_dT);
  _sp->k_from_T(T, k, dk_dT);

  // slightly higher tolerances are required for the density and thermal
  // conductivity calculations
  REL_TEST(drho_dT, drho_dT_fd, 3.0e-9);
  REL_TEST(dcp_dT, dcp_dT_fd, REL_TOL_CONSISTENCY);
  REL_TEST(dk_dT, dk_dT_fd, 2.0 * REL_TOL_CONSISTENCY);
}

TEST_F(ThermalGraphitePropertiesTest, derivativesBelow100)
{
  Real T = 95.0 + 273.15;
  Real dT = 1.0e-6 * T;

  Real drho_dT_fd = (_sp2->rho_from_T(T + dT) - _sp2->rho_from_T(T - dT)) / (2 * dT);
  Real dcp_dT_fd = (_sp2->cp_from_T(T + dT) - _sp2->cp_from_T(T - dT)) / (2 * dT);
  Real dk_dT_fd = (_sp2->k_from_T(T + dT) - _sp2->k_from_T(T - dT)) / (2 * dT);

  Real rho, drho_dT;
  Real cp, dcp_dT;
  Real k, dk_dT;

  _sp2->rho_from_T(T, rho, drho_dT);
  _sp2->cp_from_T(T, cp, dcp_dT);
  _sp2->k_from_T(T, k, dk_dT);

  // slightly higher tolerances are required for all measures
  REL_TEST(drho_dT, drho_dT_fd, 7e-9);
  REL_TEST(dcp_dT, dcp_dT_fd, 2.0 * REL_TOL_CONSISTENCY);
  REL_TEST(dk_dT, dk_dT_fd, 2.0 * REL_TOL_CONSISTENCY);
}

