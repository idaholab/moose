//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemperaturePressureFunctionFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(TemperaturePressureFunctionFluidPropertiesTest, fluidName)
{
  EXPECT_EQ(_fp->fluidName(), "TemperaturePressureFunctionFluidProperties");
}

/**
 * Verify calculation of the fluid properties
 */
TEST_F(TemperaturePressureFunctionFluidPropertiesTest, properties)
{
  const Real cv = 4186.0;

  const Real tol = REL_TOL_CONSISTENCY;
  const Real large_tol = 100 * tol;

  Real p = 8.56E7;
  Real T = 200.0;
  Real e = cv * T;
  Real v = 1 / (1400 + 2.5 * T + 32e-5 * p);
  Real h = cv * T + p / _fp->rho_from_p_T(p, T);

  // See header for expressions
  Real thermal_cond = 14 + 2e-2 * T + 3e-5 * p;
  Real visc = 1e-3 + 1e-5 * T - 3e-9 * p;
  Real density = 1 / v;
  Real dv_dT = -1 / density / density * 2.5;
  Real cp = cv + p * dv_dT;

  ABS_TEST(_fp->cp_from_p_T(p, T), cp, tol);
  ABS_TEST(_fp->cv_from_p_T(p, T), cv, tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->rho_from_p_T(p, T), density, tol);
  ABS_TEST(_fp->v_from_p_T(p, T), 1 / density, tol);
  ABS_TEST(_fp->e_from_p_T(p, T), cv * T, tol);
  ABS_TEST(_fp->e_from_p_rho(p, 1. / v),
           cv * T,
           large_tol); // uses a Newton solve for variable set inversion
  ABS_TEST(_fp->mu_from_p_T(p, T), visc, tol);
  ABS_TEST(_fp->h_from_p_T(p, T), h, tol);
  ABS_TEST(_fp->cp_from_v_e(v, e), cp, tol);
  ABS_TEST(_fp->cv_from_v_e(v, e), cv, tol);
  ABS_TEST(_fp->k_from_v_e(v, e), thermal_cond, tol);
  ABS_TEST(_fp->T_from_v_e(v, e), T, tol);
  ABS_TEST(_fp->T_from_p_h(p, _fp->h_from_p_T(p, T)), T, tol);
  ABS_TEST(_fp->T_from_p_rho(p, 1. / v), T, tol);
  ABS_TEST(
      _fp->p_from_v_e(v, e), p, 1.5 * large_tol); // uses a Newton solve for variable set inversion
  ABS_TEST(_fp->mu_from_v_e(v, e), visc, tol);

  p = 1.06841E9;
  T = 300.0;
  e = cv * T;
  v = 1 / (1400 + 2.5 * T + 32e-5 * p);
  h = cv * T + p / _fp->rho_from_p_T(p, T);

  // See header for expressions
  thermal_cond = 14 + 2e-2 * T + 3e-5 * p;
  visc = 1e-3 + 1e-5 * T - 3e-9 * p;
  density = 1 / v;
  dv_dT = -1 / density / density * 2.5;
  cp = cv + p * dv_dT;

  ABS_TEST(_fp->cp_from_p_T(p, T), cp, tol);
  ABS_TEST(_fp->cv_from_p_T(p, T), cv, tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->k_from_p_T(p, T), thermal_cond, tol);
  ABS_TEST(_fp->rho_from_p_T(p, T), density, tol);
  ABS_TEST(_fp->v_from_p_T(p, T), 1 / density, tol);
  ABS_TEST(_fp->e_from_p_T(p, T), cv * T, tol);
  ABS_TEST(_fp->e_from_p_rho(p, 1. / v),
           cv * T,
           large_tol); // uses a Newton solve for variable set inversion
  ABS_TEST(_fp->mu_from_p_T(p, T), visc, tol);
  ABS_TEST(_fp->h_from_p_T(p, T), h, tol);
  ABS_TEST(_fp->cp_from_v_e(v, e), cp, tol);
  ABS_TEST(_fp->cv_from_v_e(v, e), cv, tol);
  ABS_TEST(_fp->k_from_v_e(v, e), thermal_cond, tol);
  ABS_TEST(_fp->T_from_v_e(v, e), T, tol);
  ABS_TEST(_fp->T_from_p_h(p, _fp->h_from_p_T(p, T)), T, tol);
  ABS_TEST(_fp->T_from_p_rho(p, 1. / v), T, tol);
  ABS_TEST(_fp->p_from_v_e(v, e), p, large_tol); // uses a Newton solve for variable set inversion
  ABS_TEST(_fp->mu_from_v_e(v, e), visc, tol);
}

/**
 * Verify calculation of the derivatives by comparing with finite
 * differences
 */
TEST_F(TemperaturePressureFunctionFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;
  const Real large_tol = 30 * tol;

  // p, T and v,e are not consistent here but that's ok
  Real p = 1.0E7;
  Real T = 10.0;
  Real e = 8.372E5;
  Real v = 1.25E-3;

  AD_DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, large_tol); // uses finite differencing
  DERIV_TEST(_fp->cv_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_v_e, v, e, large_tol); // uses finite differencing
  DERIV_TEST(_fp->cv_from_v_e, v, e, tol);

  p = 5.0E7;
  T = 90.0;
  e = 1.6744E6;
  v = 6.25E-4;

  AD_DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, large_tol); // uses finite differencing
  DERIV_TEST(_fp->cv_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_v_e, v, e, large_tol); // uses finite differencing
  DERIV_TEST(_fp->cv_from_v_e, v, e, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(TemperaturePressureFunctionFluidPropertiesTest, combined)
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
