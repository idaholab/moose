//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

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

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);

  p = 5.0E7;
  T = 90.0;

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(SimpleFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;
  const Real tol = REL_TOL_CONSISTENCY;

  // Single property methods
  Real rho, drho_dp, drho_dT;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real mu, dmu_dp, dmu_dT;
  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  Real e, de_dp, de_dT;
  _fp->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT, e2, de2_dp, de2_dT;
  _fp->rho_mu_from_p_T(p, T, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  _fp->rho_mu_from_p_T(p, T, rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);

  _fp->rho_e_from_p_T(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}
