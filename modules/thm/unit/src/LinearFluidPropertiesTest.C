#include "LinearFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(LinearFluidPropertiesTest, test)
{
  const Real p = 1e5;
  const Real T = 300;

  const Real rho_from_p_T = _fp->rho_from_p_T(p, T);
  const Real rho = rho_from_p_T;

  const Real h_from_p_T = _fp->h_from_p_T(p, T);
  const Real h = h_from_p_T;

  const Real e_from_p_rho = _fp->e_from_p_rho(p, rho);
  const Real e = e_from_p_rho;

  const Real v = 1 / rho;

  // TODO: const Real s_from_v_e = _fp->s_from_v_e(v, e);
  // TODO: const Real s = s_from_v_e;

  // p
  REL_TEST(_fp->p_from_v_e(v, e), p, REL_TOL_CONSISTENCY);
  // TODO: REL_TEST(_fp->p_from_h_s(h, s), p, REL_TOL_CONSISTENCY);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->p_from_h_s);
  DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);
  // TODO: DERIV_TEST(_fp->p_from_h_s, h, s, REL_TOL_DERIVATIVE);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->p_from_h_s);

  // T
  REL_TEST(_fp->T_from_v_e(v, e), T, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);

  // rho
  // TODO: REL_TEST(rho_from_p_T, rho_saved, REL_TOL_SAVED_VALUE);
  // TODO: REL_TEST(_fp->rho_from_p_s(p, s), rho_from_p_T, REL_TOL_CONSISTENCY);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->rho_from_p_s);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
  // TODO: DERIV_TEST(_fp->rho_from_p_s, p, s, REL_TOL_DERIVATIVE);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->rho_from_p_s);

  // e
  // TODO: REL_TEST(e_from_p_rho, e_saved, REL_TOL_SAVED_VALUE);
  // TODO: REL_TEST(_fp->e_from_v_h(v, h), e, REL_TOL_CONSISTENCY);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->e_from_v_h);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, 1e-5);
  // TODO: DERIV_TEST(_fp->e_from_v_h, v, h, REL_TOL_DERIVATIVE);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->e_from_v_h);

  // c
  // TODO: const Real c = _fp->c_from_v_e(v, e);
  // TODO: REL_TEST(c, c_saved, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->c_from_v_e, v, e, REL_TOL_DERIVATIVE);

  // cp
  // TODO: const Real cp = _fp->cp_from_v_e(v, e);
  // TODO: REL_TEST(cp, cp_saved, REL_TOL_SAVED_VALUE);

  // cv
  // TODO: const Real cv = _fp->cv_from_v_e(v, e);
  // TODO: REL_TEST(cv, cv_saved, REL_TOL_SAVED_VALUE);

  // mu
  // TODO: const Real mu = _fp->mu_from_v_e(v, e);
  // TODO: REL_TEST(mu, mu_saved, REL_TOL_SAVED_VALUE);

  // k
  // TODO: const Real k = _fp->k_from_v_e(v, e);
  // TODO: REL_TEST(k, k_saved, REL_TOL_SAVED_VALUE);

  // s
  // TODO: REL_TEST(s, s_saved, REL_TOL_SAVED_VALUE);
  // TODO: REL_TEST(_fp->s_from_h_p(h, p), s, REL_TOL_CONSISTENCY);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->s_from_v_e);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->s_from_h_p);
  // TODO: DERIV_TEST(_fp->s_from_v_e, p, rho, REL_TOL_DERIVATIVE);
  // TODO: DERIV_TEST(_fp->s_from_h_p, h, p, REL_TOL_DERIVATIVE);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->s_from_v_e);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->s_from_h_p);

  // g
  // TODO: REL_TEST(_fp->g_from_v_e(v, e), g_saved, REL_TOL_SAVED_VALUE);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->g_from_v_e);

  // h
  REL_TEST(h, 300100, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->h_from_p_T, p, T, 5e-3);

  // beta
  // TODO: const Real beta = _fp->beta_from_p_T(p, T);
  // TODO: REL_TEST(beta, beta_saved, REL_TOL_SAVED_VALUE);
}
