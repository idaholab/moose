//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ThermalSS316PropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalSS316PropertiesTest, k)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp->k_from_T(T), 23.36336, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp->k_from_T(T), 18.06275, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalSS316PropertiesTest, cp)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp->cp_from_T(T), 573.74, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp->cp_from_T(T), 519.26, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific internal energy and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalSS316PropertiesTest, e)
{
  const Real T = 800.0;
  REL_TEST(_sp->e_from_T(T), 277071.479237, REL_TOL_SAVED_VALUE);
  SPECIFIC_INTERNAL_ENERGY_TESTS(_sp, T, 1e-6, 1e-6);
}

/**
 * Test that the density and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalSS316PropertiesTest, rho)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp->rho_from_T(T), 7717.1344, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->rho_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp->rho_from_T(T), 7863.415, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->rho_from_T, T, REL_TOL_DERIVATIVE);
}
