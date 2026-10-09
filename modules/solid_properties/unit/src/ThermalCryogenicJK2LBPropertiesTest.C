//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ThermalCryogenicJK2LBPropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed. Golden values calculated from log-polynomial correlations.
 */
TEST_F(ThermalCryogenicJK2LBPropertiesTest, k)
{
  Real T;

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->k_from_T(T), 5.33306828432682067e+00, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);

  // Test at mid-range temperature
  T = 150.0;
  REL_TEST(_sp->k_from_T(T), 7.49602421777116756e+00, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);

  // Test near room temperature
  T = 293.0;
  REL_TEST(_sp->k_from_T(T), 1.06265694875841046e+01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed. Golden values calculated from log-polynomial correlations.
 */
TEST_F(ThermalCryogenicJK2LBPropertiesTest, cp)
{
  Real T;

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->cp_from_T(T), 1.79487522946011552e+02, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);

  // Test at mid-range temperature
  T = 150.0;
  REL_TEST(_sp->cp_from_T(T), 3.64017706501562031e+02, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);

  // Test near room temperature
  T = 293.0;
  REL_TEST(_sp->cp_from_T(T), 4.66772622770440705e+02, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific internal energy and its derivatives are
 * correctly computed. Golden value calculated by integrating log-polynomial cp correlation.
 * e_from_T(T) = cp_integral(T) - cp_integral(T_zero_e), where T_zero_e = 273.15 K (default).
 */
TEST_F(ThermalCryogenicJK2LBPropertiesTest, e)
{
  const Real T = 100.0;
  REL_TEST(_sp->e_from_T(T), -6.89044991931350378e+04, REL_TOL_SAVED_VALUE);
  SPECIFIC_INTERNAL_ENERGY_TESTS(_sp, T, 1e-6, 1e-6);
}

/**
 * Test that the density and its derivatives are
 * correctly computed (constant density).
 */
TEST_F(ThermalCryogenicJK2LBPropertiesTest, rho)
{
  Real T;

  T = 10.0;
  REL_TEST(_sp->rho_from_T(T), 7830.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->rho_from_T, T, REL_TOL_DERIVATIVE);

  T = 293.0;
  REL_TEST(_sp->rho_from_T(T), 7830.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->rho_from_T, T, REL_TOL_DERIVATIVE);
}
