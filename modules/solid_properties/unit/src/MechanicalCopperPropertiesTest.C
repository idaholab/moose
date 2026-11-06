//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "MechanicalCopperPropertiesTest.h"

/**
 * Test Young's modulus and its derivatives at multiple temperatures.
 * Golden values generated from calculate_mechanical_golden_values.py
 */
TEST_F(MechanicalCopperPropertiesTest, E)
{
  Real T;

  // T = 77.0 K (liquid nitrogen temperature)
  T = 77.0;
  REL_TEST(_sp->E_from_T(T), 1.362470170000000e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);

  // T = 150.0 K (intermediate cryogenic temperature)
  T = 150.0;
  REL_TEST(_sp->E_from_T(T), 1.341425000000000e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);

  // T = 300.0 K (room temperature)
  T = 300.0;
  REL_TEST(_sp->E_from_T(T), 1.255700000000000e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test Poisson's ratio and its derivatives at multiple temperatures.
 * Golden values generated from calculate_mechanical_golden_values.py
 */
TEST_F(MechanicalCopperPropertiesTest, nu)
{
  Real T;

  // T = 77.0 K
  T = 77.0;
  REL_TEST(_sp->nu_from_T(T), 3.394168087000000e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->nu_from_T, T, REL_TOL_DERIVATIVE);

  // T = 150.0 K
  T = 150.0;
  REL_TEST(_sp->nu_from_T(T), 3.405817500000000e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->nu_from_T, T, REL_TOL_DERIVATIVE);

  // T = 300.0 K
  T = 300.0;
  REL_TEST(_sp->nu_from_T(T), 3.453270000000001e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->nu_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test coefficient of thermal expansion and its derivatives at multiple temperatures.
 * Golden values generated from calculate_mechanical_golden_values.py
 *
 * Note: Uses REL_TOL_CONSISTENCY (1e-10) for value tests because the logarithmic
 * correlation involves many floating-point operations that introduce rounding errors
 * beyond the 1e-12 tolerance. Uses custom perturbation (1e-4) for derivative test
 * because the logarithmic correlation is sensitive to perturbation size.
 * This matches the approach used for cp in ThermalCopperProperties (both use the
 * same NIST logarithmic polynomial form).
 */
TEST_F(MechanicalCopperPropertiesTest, alpha)
{
  Real T;

  // T = 77.0 K
  T = 77.0;
  REL_TEST(_sp->alpha_from_T(T), 7.945362590768101e-06, REL_TOL_CONSISTENCY);
  DERIV_TEST_CUSTOM_PERTURBATION(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE, 1e-4);

  // T = 150.0 K
  T = 150.0;
  REL_TEST(_sp->alpha_from_T(T), 1.357700751535986e-05, REL_TOL_CONSISTENCY);
  DERIV_TEST_CUSTOM_PERTURBATION(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE, 1e-4);

  // T = 300.0 K
  T = 300.0;
  REL_TEST(_sp->alpha_from_T(T), 1.658332498993727e-05, REL_TOL_CONSISTENCY);
  DERIV_TEST_CUSTOM_PERTURBATION(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE, 1e-4);
}
