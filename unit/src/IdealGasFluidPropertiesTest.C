//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Verify that the fluid name is correctly returned
 */
TEST_F(IdealGasFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "ideal_gas"); }

TEST_F(IdealGasFluidPropertiesTest, testAll)
{
  // Test when R and gamma are provided
  Real T = 120. + 273.15; // K
  Real p = 101325;        // Pa

  const Real rho = _fp->rho_from_p_T(p, T);
  const Real v = 1.0 / rho;
  const Real e = _fp->e_from_p_rho(p, rho);
  const Real s = _fp->s_from_v_e(v, e);
  const Real h = _fp->h_from_p_T(p, T);

  Real e_inv = _fp->e_from_T_v(T, v);
  REL_TEST(e_inv, e, 10.0 * REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->e_from_T_v, T, v, 10.0 * REL_TOL_DERIVATIVE);
  Real p_inv = _fp->p_from_T_v(T, v);
  REL_TEST(p_inv, p, 10.0 * REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->p_from_T_v, T, v, 10.0 * REL_TOL_DERIVATIVE);
  REL_TEST(_fp->h_from_T_v(T, v), h, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->h_from_T_v, T, v, 10.0 * REL_TOL_DERIVATIVE);
  REL_TEST(_fp->s_from_T_v(T, v), s, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->s_from_T_v, T, v, 10.0 * REL_TOL_DERIVATIVE);
  REL_TEST(_fp->cv_from_T_v(T, v), _fp->cv_from_v_e(v, e), REL_TOL_CONSISTENCY);

  REL_TEST(_fp->p_from_h_s(h, s), p, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->p_from_h_s, h, s, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->rho_from_p_s(p, s), rho, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->rho_from_p_s, p, s, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->p_from_v_e(v, e), p, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->T_from_v_e(v, e), T, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->c_from_v_e(v, e), 398.896207251962, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cp_from_v_e(v, e), 987.13756097561, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);
  REL_TEST(_fp->cv_from_v_e(v, e), 700.09756097561, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->mu_from_v_e(v, e), 18.23e-6, 1e-15);
  REL_TEST(_fp->k_from_v_e(v, e), 25.68e-3, 1e-15);

  REL_TEST(_fp->beta_from_p_T(p, T), 2.54355843825512e-3, REL_TOL_SAVED_VALUE);

  REL_TEST(_fp->s_from_v_e(v, e), 2.58890011905277e3, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_v_e, v, e, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->rho_from_p_T(p, T), 0.897875065343506, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->v_from_p_T(p, T), 1.0 / 0.897875065343506, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->v_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->e_from_p_rho(p, rho), 2.75243356098e5, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->h_from_p_T(p, T), 3.88093132097561e5, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->s_from_p_T(p, T), 2.588900119052767e3, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);
  ABS_TEST(_fp->s_from_h_p(h, p), 2.588900119052767e3, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_h_p, h, p, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->e_from_p_T(p, T), 2.75243356097561e5, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->e_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->molarMass(), 0.0289662061037, REL_TOL_SAVED_VALUE);

  ABS_TEST(_fp->T_from_p_h(p, h), T, REL_TOL_CONSISTENCY);

  REL_TEST(_fp->mu_from_p_T(p, T), 18.23e-6, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->k_from_p_T(p, T), 25.68e-3, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->cv_from_p_T(p, T), 700.09756097561, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cp_from_p_T(p, T), 987.13756097561, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);

  // Test when R and gamma are not provided
  const Real molar_mass = 0.029;
  const Real cv = 718.0;
  const Real cp = 1005.0;
  const Real thermal_conductivity = 0.02568;
  const Real entropy = 1767.24985689;
  const Real viscosity = 18.23e-6;
  Real henry = 0.0;
  const Real R = 8.3144598;

  const Real tol = REL_TOL_CONSISTENCY;

  p = 1.0e6;
  T = 300.0;

  REL_TEST(_fp_pT->beta_from_p_T(p, T), 1.0 / T, tol);
  REL_TEST(_fp_pT->cp_from_p_T(p, T), cp, tol);
  REL_TEST(_fp_pT->cv_from_p_T(p, T), cv, tol);
  REL_TEST(_fp_pT->c_from_p_T(p, T), std::sqrt(cp * R * T / (cv * molar_mass)), tol);
  REL_TEST(_fp_pT->k_from_p_T(p, T), thermal_conductivity, tol);
  REL_TEST(_fp_pT->k_from_p_T(p, T), thermal_conductivity, tol);
  REL_TEST(_fp_pT->s_from_p_T(p, T), entropy, tol);

  REL_TEST(_fp_pT->rho_from_p_T(p, T), p * molar_mass / (R * T), tol);
  REL_TEST(_fp_pT->e_from_p_T(p, T), cv * T, tol);
  REL_TEST(_fp_pT->mu_from_p_T(p, T), viscosity, tol);
  REL_TEST(_fp_pT->mu_from_p_T(p, T), viscosity, tol);
  REL_TEST(_fp_pT->h_from_p_T(p, T), cp * T, tol);
  ABS_TEST(_fp_pT->henryConstant(T), henry, tol);

  DERIV_TEST(_fp_pT->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp_pT->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp_pT->e_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp_pT->h_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp_pT->k_from_p_T, p, T, REL_TOL_DERIVATIVE);

  Real dhenry_dT;
  _fp_pT->henryConstant(T, henry, dhenry_dT);
  ABS_TEST(dhenry_dT, 0.0, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(IdealGasFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;
  const Real tol = REL_TOL_CONSISTENCY;

  // Single property methods
  Real rho, drho_dp, drho_dT;
  _fp_pT->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real mu, dmu_dp, dmu_dT;
  _fp_pT->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  Real e, de_dp, de_dT;
  _fp_pT->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT, e2, de2_dp, de2_dT;
  _fp_pT->rho_mu_from_p_T(p, T, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  _fp_pT->rho_mu_from_p_T(p, T, rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);

  _fp_pT->rho_e_from_p_T(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}
