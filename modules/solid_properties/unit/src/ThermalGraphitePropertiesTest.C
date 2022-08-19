//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ThermalGraphitePropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalGraphitePropertiesTest, k)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp1->k_from_T(T), 90.31037199999997, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->k_from_T(T), 114.97569999999999, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalGraphitePropertiesTest, cp)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp1->cp_from_T(T), 1619.4403751760003, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->cp_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->cp_from_T(T), 1217.6343116400003, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->cp_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the density and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalGraphitePropertiesTest, rho)
{
  Real T = 800.0;

  REL_TEST(_sp1->rho_from_T(T), 1850.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->rho_from_T, T, REL_TOL_DERIVATIVE);

  REL_TEST(_sp2->rho_from_T(T), 2000.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp2->rho_from_T, T, REL_TOL_DERIVATIVE);
}
