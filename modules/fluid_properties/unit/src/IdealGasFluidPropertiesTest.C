//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  const Real T = 120. + 273.15; // K
  const Real p = 101325;        // Pa

  // saved values
  const Real rho = 0.897875065343506;
  const Real v = 1.1137406957809;
  const Real e = 2.762433560975611e+05;
  const Real h = 3.890931320975610e+05;
  const Real s = 2588.90011905277;
  const Real c = 398.896207251962;
  const Real cp = 987.13756097561;
  const Real cv = 700.09756097561;
  const Real mu = 1.823000000000000e-05;
  const Real k = 0.02568;
  const Real beta = 0.00254355843825512;

  // Because the ideal gas equation of state is exact, we expect consistency
  // to roundoff error:
  const Real rel_tol_consistency = REL_TOL_SAVED_VALUE;

  REL_TEST(_fp->T_from_v_e(v, e), T, rel_tol_consistency);
  REL_TEST(_fp->T_from_p_h(p, h), T, rel_tol_consistency);
  DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->T_from_p_h, p, h, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->p_from_v_e(v, e), p, rel_tol_consistency);
  REL_TEST(_fp->p_from_T_v(T, v), p, rel_tol_consistency);
  REL_TEST(_fp->p_from_h_s(h, s), p, rel_tol_consistency);
  DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->p_from_T_v, T, v, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->p_from_h_s, h, s, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->rho_from_p_T(p, T), rho, rel_tol_consistency);
  REL_TEST(_fp->rho_from_p_s(p, s), rho, rel_tol_consistency);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->rho_from_p_s, p, s, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->v_from_p_T(p, T), v, rel_tol_consistency);
  DERIV_TEST(_fp->v_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->e_from_p_rho(p, rho), e, rel_tol_consistency);
  REL_TEST(_fp->e_from_p_T(p, T), e, rel_tol_consistency);
  REL_TEST(_fp->e_from_T_v(T, v), e, rel_tol_consistency);
  REL_TEST(_fp->e_from_v_h(v, h), e, rel_tol_consistency);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->e_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->e_from_T_v, T, v, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->e_from_v_h, v, h, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->h_from_p_T(p, T), h, rel_tol_consistency);
  REL_TEST(_fp->h_from_T_v(T, v), h, rel_tol_consistency);
  DERIV_TEST(_fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->h_from_T_v, T, v, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->s_from_v_e(v, e), s, rel_tol_consistency);
  REL_TEST(_fp->s_from_p_T(p, T), s, rel_tol_consistency);
  REL_TEST(_fp->s_from_T_v(T, v), s, rel_tol_consistency);
  REL_TEST(_fp->s_from_h_p(h, p), s, rel_tol_consistency);
  DERIV_TEST(_fp->s_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->s_from_T_v, T, v, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->s_from_h_p, h, p, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->c_from_v_e(v, e), c, rel_tol_consistency);
  REL_TEST(_fp->c_from_p_T(p, T), c, rel_tol_consistency);
  DERIV_TEST(_fp->c_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->c_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->cp_from_v_e(v, e), cp, rel_tol_consistency);
  REL_TEST(_fp->cp_from_p_T(p, T), cp, rel_tol_consistency);
  DERIV_TEST(_fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->cv_from_v_e(v, e), cv, rel_tol_consistency);
  REL_TEST(_fp->cv_from_p_T(p, T), cv, rel_tol_consistency);
  REL_TEST(_fp->cv_from_T_v(T, v), cv, rel_tol_consistency);
  DERIV_TEST(_fp->cv_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->cv_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->mu_from_v_e(v, e), mu, rel_tol_consistency);
  REL_TEST(_fp->mu_from_p_T(p, T), mu, rel_tol_consistency);
  DERIV_TEST(_fp->mu_from_v_e, p, T, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->k_from_v_e(v, e), k, rel_tol_consistency);
  REL_TEST(_fp->k_from_p_T(p, T), k, rel_tol_consistency);
  DERIV_TEST(_fp->k_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->beta_from_p_T(p, T), beta, rel_tol_consistency);
  DERIV_TEST(_fp->beta_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->molarMass(), _molar_mass, rel_tol_consistency);
}
