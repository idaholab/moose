//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidPropertiesTest.h"

TEST_F(IdealGasFluidPropertiesTest, testAll)
{
  const Real T = 120. + 273.15; // K
  const Real p = 101325;        // Pa

  const Real rho = _fp->rho_from_p_T(p, T);
  const Real v = 1 / rho;
  const Real e = _fp->e_from_p_rho(p, rho);
  const Real s = _fp->s_from_v_e(v, e);
  const Real h = _fp->h_from_p_T(p, T);

  REL_TEST(_fp->p_from_h_s(h, s), p, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->p_from_h_s, h, s, REL_TOL_DERIVATIVE);

  REL_TEST(rho, 0.897875065343506, 1e-15);
  REL_TEST(_fp->rho_from_p_s(p, s), rho, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->rho_from_p_s, p, s, REL_TOL_DERIVATIVE);

  REL_TEST(e, 275243.356097561, 1e-15);

  REL_TEST(_fp->p_from_v_e(v, e), p, 1e-15);
  REL_TEST(_fp->T_from_v_e(v, e), T, 1e-15);
  REL_TEST(_fp->c_from_v_e(v, e), 398.896207251962, 1e-15);
  REL_TEST(_fp->cp_from_v_e(v, e), 987.13756097561, 1e-15);
  REL_TEST(_fp->cv_from_v_e(v, e), 700.09756097561, 1e-15);

  REL_TEST(h, 388093, 4e-7);

  REL_TEST(s, 2588.90011905277, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->s_from_v_e, v, e, REL_TOL_DERIVATIVE);

  // derivatives

  {
    const Real de = 1e-3;
    const Real dv = 1e-6;

    Real dp_dv_fd = (_fp->p_from_v_e(v + dv, e) - _fp->p_from_v_e(v - dv, e)) / (2 * dv);
    Real dp_de_fd = (_fp->p_from_v_e(v, e + de) - _fp->p_from_v_e(v, e - de)) / (2 * de);

    Real dT_dv_fd = (_fp->T_from_v_e(v + dv, e) - _fp->T_from_v_e(v - dv, e)) / (2 * dv);
    Real dT_de_fd = (_fp->T_from_v_e(v, e + de) - _fp->T_from_v_e(v, e - de)) / (2 * de);

    Real p, dp_dv, dp_de;
    _fp->p_from_v_e(v, e, p, dp_dv, dp_de);

    Real T, dT_dv, dT_de;
    _fp->T_from_v_e(v, e, T, dT_dv, dT_de);

    ABS_TEST(dp_dv, dp_dv_fd, 4e-6);
    ABS_TEST(dp_de, dp_de_fd, 1e-8);
    ABS_TEST(dT_dv, dT_dv_fd, 1e-15);
    ABS_TEST(dT_de, dT_de_fd, 1e-11);
  }

  {
    const Real p = 1e6;
    const Real T = 500;
    const Real dp = 1e1;
    const Real dT = 1e-4;

    // density
    Real drho_dp_fd = (_fp->rho_from_p_T(p + dp, T) - _fp->rho_from_p_T(p - dp, T)) / (2 * dp);
    Real drho_dT_fd = (_fp->rho_from_p_T(p, T + dT) - _fp->rho_from_p_T(p, T - dT)) / (2 * dT);
    Real rho = 0, drho_dp = 0, drho_dT = 0;
    _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);

    ABS_TEST(rho, _fp->rho_from_p_T(p, T), 1e-15);
    ABS_TEST(drho_dp, drho_dp_fd, 1e-15);
    ABS_TEST(drho_dT, drho_dT_fd, 1e-11);
  }

  {
    const Real p = 1e6;
    const Real rho = _fp->rho_from_p_T(p, 500);
    const Real dp = 1e1;
    const Real drho = 1e-4;

    // internal energy
    Real de_dp_fd = (_fp->e_from_p_rho(p + dp, rho) - _fp->e_from_p_rho(p - dp, rho)) / (2 * dp);
    Real de_drho_fd =
        (_fp->e_from_p_rho(p, rho + drho) - _fp->e_from_p_rho(p, rho - drho)) / (2 * drho);
    Real e, de_dp, de_drho;
    _fp->e_from_p_rho(p, rho, e, de_dp, de_drho);

    ABS_TEST(e, _fp->e_from_p_rho(p, rho), 1e-16);
    ABS_TEST(de_dp, de_dp_fd, 5e-11);
    ABS_TEST(de_drho, de_drho_fd, 5e-5);
  }

  {
    const Real p = 1e6;
    const Real T = 500;
    const Real dp = 1e1;
    const Real dT = 1e-4;

    // enthalpy
    Real dh_dp_fd = (_fp->h_from_p_T(p + dp, T) - _fp->h_from_p_T(p - dp, T)) / (2 * dp);
    Real dh_dT_fd = (_fp->h_from_p_T(p, T + dT) - _fp->h_from_p_T(p, T - dT)) / (2 * dT);

    Real h = 0, dh_dp = 0, dh_dT = 0;
    _fp->h_from_p_T(p, T, h, dh_dp, dh_dT);

    ABS_TEST(h, _fp->h_from_p_T(p, T), 1e-16);
    ABS_TEST(dh_dp, dh_dp_fd, 1e-12);
    ABS_TEST(dh_dT, dh_dT_fd, 6e-7);
  }

  {
    // entropy from enthalpy and pressure
    const Real rel_diff = 1e-6;
    const Real h = 1e5;
    const Real p = 1e5;
    const Real dh = h * rel_diff;
    const Real dp = p * rel_diff;

    Real s, ds_dh, ds_dp;
    _fp->s_from_h_p(h, p, s, ds_dh, ds_dp);

    Real s_dh, s_dp, ds_dh_dummy, ds_dp_dummy;
    _fp->s_from_h_p(h + dh, p, s_dh, ds_dh_dummy, ds_dp_dummy);
    _fp->s_from_h_p(h, p + dp, s_dp, ds_dh_dummy, ds_dp_dummy);

    const Real ds_dh_fd = (s_dh - s) / dh;
    const Real ds_dp_fd = (s_dp - s) / dp;

    const Real rel_tol = 1e-5;
    REL_TEST(ds_dh, ds_dh_fd, rel_tol);
    REL_TEST(ds_dp, ds_dp_fd, rel_tol);
  }
}
