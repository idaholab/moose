//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ThermalCompositeSiCPropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalCompositeSiCPropertiesTest, k)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp1->k_from_T(T), 16.997839999999997, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->k_from_T(T), 19.088749999999997, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalCompositeSiCPropertiesTest, cp)
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
 * Test that the specific internal energy and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalCompositeSiCPropertiesTest, e)
{
  const Real T = 800.0;
  REL_TEST(_sp1->e_from_T(T), 504301.09074042423, REL_TOL_SAVED_VALUE);
  SPECIFIC_INTERNAL_ENERGY_TESTS(_sp1, T, 1e-6, 1e-6);
}

/**
 * Test that the density and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalCompositeSiCPropertiesTest, rho)
{
  Real T = 800.0;

  REL_TEST(_sp1->rho_from_T(T), 3216.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->rho_from_T, T, REL_TOL_DERIVATIVE);

  REL_TEST(_sp2->rho_from_T(T), 3000.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp2->rho_from_T, T, REL_TOL_DERIVATIVE);
}
