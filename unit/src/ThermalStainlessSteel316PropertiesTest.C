//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalStainlessSteel316PropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(ThermalStainlessSteel316PropertiesTest, name)
{
  EXPECT_EQ(_sp->solidName(), "thermal_stainless_steel_316");
}

TEST_F(ThermalStainlessSteel316PropertiesTest, molarMass)
{
  EXPECT_EQ(_sp->molarMass(), 55.34175166872447e-3);
}

TEST_F(ThermalStainlessSteel316PropertiesTest, values)
{
  Real T = 1000.0 + 273.15;

  REL_TEST(_sp->cp_from_T(T), 659.66404, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->k_from_T(T), 29.0524833548275, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->rho_from_T(T), 7470.132072511849, REL_TOL_SAVED_VALUE);
  ABS_TEST(_sp->emissivity_from_T(T), 0.7, REL_TOL_SAVED_VALUE);
  ABS_TEST(_sp2->emissivity_from_T(T), 0.15, REL_TOL_SAVED_VALUE);
  ABS_TEST(_sp3->emissivity_from_T(T), 0.8, REL_TOL_SAVED_VALUE);
}

TEST_F(ThermalStainlessSteel316PropertiesTest, derivatives)
{
  Real T = 1000.0 + 273.15;
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

  REL_TEST(drho_dT, drho_dT_fd, REL_TOL_CONSISTENCY);
  REL_TEST(dcp_dT, dcp_dT_fd, REL_TOL_CONSISTENCY);
  REL_TEST(dk_dT, dk_dT_fd, 3.0 * REL_TOL_CONSISTENCY);
}
