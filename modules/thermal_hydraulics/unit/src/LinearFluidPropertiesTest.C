//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  REL_TEST(_fp->e_from_p_T(p, T), e, REL_TOL_CONSISTENCY);
  REL_TEST(_fp->e_from_p_rho(p, rho), e, REL_TOL_CONSISTENCY);
  REL_TEST(_fp->e_from_v_h(v, h), e, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->e_from_p_T, p, T, 1e-5);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, 1e-5);
  DERIV_TEST(_fp->e_from_v_h, v, h, REL_TOL_DERIVATIVE);

  // c
  // TODO: const Real c = _fp->c_from_v_e(v, e);
  // TODO: REL_TEST(c, c_saved, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->c_from_v_e, v, e, REL_TOL_DERIVATIVE);

  // cp
  const Real cp = _fp->cp_from_v_e(v, e);
  REL_TEST(cp, 1000, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);

  // cv
  const Real cv = _fp->cv_from_v_e(v, e);
  REL_TEST(cv, 1000, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cv_from_v_e, v, e, REL_TOL_DERIVATIVE);

  // mu
  const Real mu = _fp->mu_from_v_e(v, e);
  REL_TEST(mu, 0.3, REL_TOL_SAVED_VALUE);

  // k
  const Real k = _fp->k_from_v_e(v, e);
  REL_TEST(k, 0.89, REL_TOL_SAVED_VALUE);

  // s
  // TODO: REL_TEST(s, s_saved, REL_TOL_SAVED_VALUE);
  // TODO: REL_TEST(_fp->s_from_h_p(h, p), s, REL_TOL_CONSISTENCY);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->s_from_p_T);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->s_from_v_e);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->s_from_h_p);
  // TODO: DERIV_TEST(_fp->s_from_v_e, p, rho, REL_TOL_DERIVATIVE);
  // TODO: DERIV_TEST(_fp->s_from_h_p, h, p, REL_TOL_DERIVATIVE);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->s_from_p_T);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->s_from_v_e);
  NOT_IMPLEMENTED_TEST_DERIV(_fp->s_from_h_p);

  // g
  // TODO: REL_TEST(_fp->g_from_v_e(v, e), g_saved, REL_TOL_SAVED_VALUE);
  NOT_IMPLEMENTED_TEST_VALUE(_fp->g_from_v_e);

  // h
  REL_TEST(h, 1000100, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->h_from_p_T, p, T, 5e-3);

  // beta
  const Real beta = _fp->beta_from_p_T(p, T);
  REL_TEST(beta, 123, REL_TOL_SAVED_VALUE);

  // molar mass
  try
  {
    _fp->molarMass();
  }
  catch (const std::exception & x)
  {
    std::string msg(x.what());
    EXPECT_TRUE(msg.find("not implemented") != std::string::npos);
  }

  // Pr
  const Real Pr = _fp->Pr(rho, T);
  REL_TEST(Pr, (1000 * 0.3) / 0.89, REL_TOL_SAVED_VALUE);
}
