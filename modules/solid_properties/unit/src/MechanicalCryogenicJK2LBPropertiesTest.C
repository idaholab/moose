//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "MechanicalCryogenicJK2LBPropertiesTest.h"

/**
 * Test that the Young's modulus and its derivatives are
 * correctly computed. Golden values calculated from linear correlation.
 */
TEST_F(MechanicalCryogenicJK2LBPropertiesTest, E)
{
  Real T;

  // Test at 10 K
  T = 10.0;
  REL_TEST(_sp->E_from_T(T), 1.99833910034602051e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->E_from_T(T), 1.97979238754325256e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);

  // Test at mid-range temperature
  T = 150.0;
  REL_TEST(_sp->E_from_T(T), 1.95958477508650513e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);

  // Test at room temperature
  T = 293.0;
  REL_TEST(_sp->E_from_T(T), 1.92000000000000000e+11, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->E_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the Poisson's ratio and its derivatives are
 * correctly computed. Golden value is constant from Lu et al. (2009).
 */
TEST_F(MechanicalCryogenicJK2LBPropertiesTest, nu)
{
  Real T;

  // Test at low temperature
  T = 10.0;
  REL_TEST(_sp->nu_from_T(T), 2.68000000000000016e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->nu_from_T, T, REL_TOL_DERIVATIVE);

  // Test at liquid nitrogen temperature
  T = 77.0;
  REL_TEST(_sp->nu_from_T(T), 2.68000000000000016e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->nu_from_T, T, REL_TOL_DERIVATIVE);

  // Test at room temperature
  T = 293.0;
  REL_TEST(_sp->nu_from_T(T), 2.68000000000000016e-01, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->nu_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the thermal expansion coefficient and its derivatives are
 * correctly computed. Golden values calculated from quintic polynomial fit.
 */
TEST_F(MechanicalCryogenicJK2LBPropertiesTest, alpha)
{
  Real T;

  // Test at 10 K (low temperature, negative alpha - contraction)
  T = 10.0;
  REL_TEST(_sp->alpha_from_T(T), -3.68523756131701775e-05, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE);

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->alpha_from_T(T), 3.08611562832815487e-04, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE);

  // Test at mid-range temperature
  T = 150.0;
  REL_TEST(_sp->alpha_from_T(T), 7.23522140975802960e-04, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE);

  // Test at room temperature
  T = 293.0;
  REL_TEST(_sp->alpha_from_T(T), 1.88857135999570373e-03, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->alpha_from_T, T, REL_TOL_DERIVATIVE);
}
