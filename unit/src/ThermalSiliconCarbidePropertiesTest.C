//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSiliconCarbidePropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(ThermalSiliconCarbidePropertiesTest, name)
{
  EXPECT_EQ(_sp->solidName(), "thermal_silicon_carbide");
}

TEST_F(ThermalSiliconCarbidePropertiesTest, values)
{
  Real T = 650.0 + 273.15;

  REL_TEST(_sp->cp_from_T(T), 1168.8309457900816, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->k_from_T(T), 106.46140906997975, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp->rho_from_T(T), 3216.0, REL_TOL_SAVED_VALUE);

  REL_TEST(_sp2->k_from_T(T), 21.37388290093701, REL_TOL_SAVED_VALUE);
  REL_TEST(_sp2->rho_from_T(T), 3180.0, REL_TOL_SAVED_VALUE);
}

TEST_F(ThermalSiliconCarbidePropertiesTest, derivatives)
{
  Real T = 650.0 + 273.15;
  Real dT = 1.0e-6 * T;

  Real drho_dT_fd = (_sp->rho_from_T(T + dT) - _sp->rho_from_T(T - dT)) / (2 * dT);
  Real dcp_dT_fd = (_sp->cp_from_T(T + dT) - _sp->cp_from_T(T - dT)) / (2 * dT);
  Real dk_dT_fd = (_sp->k_from_T(T + dT) - _sp->k_from_T(T - dT)) / (2 * dT);
  Real dk2_dT_fd = (_sp2->k_from_T(T + dT) - _sp2->k_from_T(T - dT)) / (2 * dT);

  Real rho, drho_dT;
  Real cp, dcp_dT;
  Real k, dk_dT;
  Real k2, dk2_dT;

  _sp->rho_from_T(T, rho, drho_dT);
  _sp->cp_from_T(T, cp, dcp_dT);
  _sp->k_from_T(T, k, dk_dT);
  _sp2->k_from_T(T, k2, dk2_dT);

  REL_TEST(drho_dT, drho_dT_fd, REL_TOL_CONSISTENCY);
  REL_TEST(dcp_dT, dcp_dT_fd, 3.0 * REL_TOL_CONSISTENCY);
  REL_TEST(dk_dT, dk_dT_fd, REL_TOL_CONSISTENCY);
  REL_TEST(dk2_dT, dk2_dT_fd, REL_TOL_CONSISTENCY);
}
