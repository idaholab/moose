//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ThermalMonolithicSiCPropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalMonolithicSiCPropertiesTest, k)
{
  Real T;

  // Snead correlation
  T = 800.0;
  REL_TEST(_sp1->k_from_T(T), 123.4567901234568, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->k_from_T(T), 202.02020202020205, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);

  // Stone correlation
  T = 800.0;
  REL_TEST(_sp2->k_from_T(T), 61.516, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp2->k_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp2->k_from_T(T), 79.975, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp2->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalMonolithicSiCPropertiesTest, cp)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp1->cp_from_T(T), 1126.7686149999997, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->cp_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->cp_from_T(T), 966.65125, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->cp_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the density and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalMonolithicSiCPropertiesTest, rho)
{
  Real T = 800.0;

  REL_TEST(_sp1->rho_from_T(T), 3216.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->rho_from_T, T, REL_TOL_DERIVATIVE);

  REL_TEST(_sp2->rho_from_T(T), 3000.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp2->rho_from_T, T, REL_TOL_DERIVATIVE);
}
