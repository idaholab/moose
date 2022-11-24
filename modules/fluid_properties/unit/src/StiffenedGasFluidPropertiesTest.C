//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StiffenedGasFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(StiffenedGasFluidPropertiesTest, testAllReal)
{
  const Real T = 20. + 273.15; // K
  const Real p = 101325;       // Pa

  const Real rho = _fp->rho_from_p_T(p, T);
  const Real v = 1 / rho;
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
  DERIV_TEST(_fp->rho_from_p_s, p, s, 1e-5);

  REL_TEST(_fp->p_from_v_e(v, e), p, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->T_from_v_e(v, e), T, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->c_from_v_e(v, e), 1.299581997797754e3, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cp_from_v_e(v, e), 4267.6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);
  REL_TEST(_fp->cv_from_v_e(v, e), 1816, REL_TOL_SAVED_VALUE);

  REL_TEST(_fp->mu_from_v_e(v, e), 0.001, 1e-15);
  DERIV_TEST(_fp->mu_from_v_e, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->k_from_v_e(v, e), 0.6, 1e-15);

  REL_TEST(_fp->beta_from_p_T(p, T), 3.411222923418045e-3, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->beta_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->s_from_v_e(v, e), -2.656251807629821e4, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_v_e, v, e, 1e-5);

  ABS_TEST(_fp->rho_from_p_T(p, T), 1.391568186319449e3, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->v_from_p_T(p, T), 1.0 / 1.391568186319449e3, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->v_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->e_from_p_rho(p, rho), 8.397412646416598e4, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->h_from_p_T(p, T), 8.404693999999994e4, 1e-10);
  DERIV_TEST(_fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->s_from_p_T(p, T), -2.6562518076298216e4, 4 * REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);
  ABS_TEST(_fp->s_from_h_p(h, p), -2.6562518076298216e4, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_h_p, h, p, 1e-5);

  ABS_TEST(_fp->e_from_p_T(p, T), 8.397412646416575e4, 1e-10);
  DERIV_TEST(_fp->e_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->e_from_v_h(v, h), e, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->e_from_v_h, v, h, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->T_from_p_h(p, h), T, REL_TOL_CONSISTENCY);
  DERIV_TEST_CUSTOM_PERTURBATION(_fp->T_from_p_h, p, h, REL_TOL_DERIVATIVE, 1e-3);

  REL_TEST(_fp->mu_from_p_T(p, T), 0.001, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->k_from_p_T(p, T), 0.6, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->cv_from_p_T(p, T), 1816, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cp_from_p_T(p, T), 4267.6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

TEST_F(StiffenedGasFluidPropertiesTest, testAllADReal)
{
  const ADReal T = 20. + 273.15; // K
  const ADReal p = 101325;       // Pa

  const ADReal rho = _fp->rho_from_p_T(p, T);
  const ADReal v = 1 / rho;
  const ADReal e = _fp->e_from_p_rho(p, rho);
  const ADReal s = _fp->s_from_v_e(v, e);

  REL_TEST(_fp->p_from_v_e(v, e), p, REL_TOL_CONSISTENCY);
  AD_DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->T_from_v_e(v, e), T, REL_TOL_SAVED_VALUE);
  AD_DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->s_from_v_e(v, e), -2.656251807629821e4, REL_TOL_SAVED_VALUE);
  AD_DERIV_TEST(_fp->s_from_v_e, v, e, 1e-5);

  ABS_TEST(_fp->s_from_p_T(p, T), -2.6562518076298216e4, 4 * REL_TOL_SAVED_VALUE);
  AD_DERIV_TEST(_fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->rho_from_p_T(p, T), 1.391568186319449e3, REL_TOL_SAVED_VALUE);
  AD_DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);

  ABS_TEST(_fp->e_from_p_rho(p, rho), 8.397412646416598e4, REL_TOL_SAVED_VALUE);
  AD_DERIV_TEST(_fp->e_from_p_rho, p, rho, REL_TOL_DERIVATIVE);
}
