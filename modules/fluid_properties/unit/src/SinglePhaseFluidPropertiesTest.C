//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(SinglePhaseFluidPropertiesTest, testAll)
{
  const Real p = 1.5;
  const Real T = 3.1;
  const Real v = _fp->v_from_p_T(p, T);
  const Real e = _fp->e_from_p_T(p, T);

  {
    Real v, dv_dp, dv_dT;
    _fp->v_from_p_T(p, T, v, dv_dp, dv_dT);

    Real e, de_dp, de_dT;
    _fp->e_from_p_T(p, T, e, de_dp, de_dT);

    Real v2, dv_dp2, dv_dT2, e2, de_dp2, de_dT2;
    _fp->v_e_from_p_T(p, T, v2, dv_dp2, dv_dT2, e2, de_dp2, de_dT2);
    REL_TEST(v2, v, REL_TOL_CONSISTENCY);
    REL_TEST(e2, e, REL_TOL_CONSISTENCY);
    REL_TEST(dv_dp2, dv_dp, REL_TOL_CONSISTENCY);
    REL_TEST(dv_dT2, dv_dT, REL_TOL_CONSISTENCY);
    REL_TEST(de_dp2, de_dp, REL_TOL_CONSISTENCY);
    REL_TEST(de_dT2, de_dT, REL_TOL_CONSISTENCY);
  }

  REL_TEST(_fp->s_from_p_T(p, T), _fp->s_from_v_e(v, e), REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->c_from_p_T(p, T), _fp->c_from_v_e(v, e), REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->c_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->mu_from_p_T(p, T), _fp->mu_from_v_e(v, e), REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->cv_from_p_T(p, T), _fp->cv_from_v_e(v, e), REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->cv_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->cp_from_p_T(p, T), _fp->cp_from_v_e(v, e), REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);

  REL_TEST(_fp->k_from_p_T(p, T), _fp->k_from_v_e(v, e), REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);
}
