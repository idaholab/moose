//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesPTTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(SimpleFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "simple_fluid"); }

/**
 * Test that the default molar mass is correctly returned
 */
TEST_F(SimpleFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 1.8e-2, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the fluid properties
 */
TEST_F(SimpleFluidPropertiesTest, properties)
{
  const Real thermal_exp = 2.14E-4;
  const Real cv = 4186.0;
  const Real cp = 4194.0;
  const Real bulk_modulus = 2.0E9;
  const Real thermal_cond = 0.6;
  const Real entropy = 300.0;
  const Real visc = 1.0E-3;
  const Real density0 = 1000.0;
  const Real henry = 0.0;
  const Real pp_coef = 0.0;

  const Real tol = REL_TOL_CONSISTENCY;

  Real p = 1.0E3;
  Real T = 200.0;

  ABS_TEST(_fp->beta_from_p_T(p, T), thermal_exp, tol);
  ABS_TEST(_fp->cp_from_p_T(p, T), cp, tol);
  ABS_TEST(_fp->cv_from_p_T(p, T), cv, tol);
  ABS_TEST(_fp->c_from_p_T(p, T), std::sqrt(bulk_modulus / _fp->rho_from_p_T(p, T)), tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->s_from_p_T(p, T), entropy, tol);
  ABS_TEST(_fp->rho_from_p_T(p, T), density0 * std::exp(p / bulk_modulus - thermal_exp * T), tol);
  ABS_TEST(_fp->e_from_p_T(p, T), cv * T, tol);
  ABS_TEST(_fp->mu_from_p_T(p, T), visc, tol);
  ABS_TEST(_fp->mu_from_p_T(p, T), visc, tol);
  ABS_TEST(_fp->h_from_p_T(p, T), cv * T + p / _fp->rho_from_p_T(p, T), tol);
  ABS_TEST(_fp2->h_from_p_T(p, T), cv * T + p * pp_coef / _fp2->rho_from_p_T(p, T), tol);
  ABS_TEST(_fp->henryConstant(T), henry, tol);

  p = 1.0E7;
  T = 300.0;
  ABS_TEST(_fp->beta_from_p_T(p, T), thermal_exp, tol);
  ABS_TEST(_fp->cp_from_p_T(p, T), cp, tol);
  ABS_TEST(_fp->cv_from_p_T(p, T), cv, tol);
  ABS_TEST(_fp->c_from_p_T(p, T), std::sqrt(bulk_modulus / _fp->rho_from_p_T(p, T)), tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->s_from_p_T(p, T), entropy, tol);
  ABS_TEST(_fp->rho_from_p_T(p, T), density0 * std::exp(p / bulk_modulus - thermal_exp * T), tol);
  ABS_TEST(_fp->e_from_p_T(p, T), cv * T, tol);
  ABS_TEST(_fp->mu_from_p_T(p, T), visc, tol);
  ABS_TEST(_fp->mu_from_p_T(p, T), visc, tol);
  ABS_TEST(_fp->h_from_p_T(p, T), cv * T + p / _fp->rho_from_p_T(p, T), tol);
  ABS_TEST(_fp2->h_from_p_T(p, T), cv * T + p * pp_coef / _fp2->rho_from_p_T(p, T), tol);
  ABS_TEST(_fp->henryConstant(T), henry, tol);
}

/**
 * Verify calculation of the derivatives by comparing with finite
 * differences
 */
TEST_F(SimpleFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  Real p = 1.0E7;
  Real T = 10.0;

  DERIV_TEST(_fp->rho, _fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu, _fp->mu_dpT, p, T, tol);
  DERIV_TEST(_fp->e, _fp->e_dpT, p, T, tol);
  DERIV_TEST(_fp->h, _fp->h_dpT, p, T, tol);
  DERIV_TEST(_fp->k, _fp->k_dpT, p, T, tol);

  p = 5.0E7;
  T = 90.0;

  DERIV_TEST(_fp->rho, _fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu, _fp->mu_dpT, p, T, tol);
  DERIV_TEST(_fp->e, _fp->e_dpT, p, T, tol);
  DERIV_TEST(_fp->h, _fp->h_dpT, p, T, tol);
  DERIV_TEST(_fp->k, _fp->k_dpT, p, T, tol);

  Real henry, dhenry_dT;
  _fp->henryConstant_dT(T, henry, dhenry_dT);
  ABS_TEST(dhenry_dT, 0.0, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(SimpleFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;

  combinedProperties(_fp, p, T, REL_TOL_SAVED_VALUE);
}
