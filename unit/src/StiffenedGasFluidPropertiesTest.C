//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StiffenedGasFluidPropertiesTest.h"

TEST_F(StiffenedGasFluidPropertiesTest, testAll)
{
  Real T = 20. + 273.15; // K
  Real p = 101325;       // Pa

  Real rho = _fp->rho(p, T);
  Real v = 1 / rho;
  Real e;

  REL_TEST("rho", _fp->rho(p, T), 1391.568186, 3e-10);
  REL_TEST("e", _fp->e(p, rho), 83974.12646, 5e-11);

  _fp->rho_e(p, T, rho, e);
  REL_TEST("rho", rho, 1391.568186, 3e-10);
  REL_TEST("e", e, 83974.12646, 5e-11);

  REL_TEST("p", _fp->pressure(v, e), p, 1e-11);
  REL_TEST("T", _fp->temperature(v, e), T, 1e-15);
  REL_TEST("c", _fp->c(v, e), 1299.581998, 2e-10);
  REL_TEST("cp", _fp->cp(v, e), 4267.6, 1e-15);
  REL_TEST("cv", _fp->cv(v, e), 1816, 1e-15);
  REL_TEST("mu", _fp->mu(v, e), 0.001, 1e-15);
  REL_TEST("k", _fp->k(v, e), 0.6, 1e-15);
  REL_TEST("s", _fp->s(v, e), -26562.51808, 5e-10);

  REL_TEST("h", _fp->h(p, T), 84046.94, 1e-15);

  // derivatives
  p = 1e6;
  T = 300;

  Real dp = 1e1;
  Real dT = 1e-4;

  {
    // density
    Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2 * dp);
    Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2 * dT);
    Real drho_dp = 0, drho_dT = 0;
    _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

    ABS_TEST("rho", rho, _fp->rho(p, T), 1e-16);
    ABS_TEST("drho_dp", drho_dp, drho_dp_fd, 2e-14);
    ABS_TEST("drho_dT", drho_dT, drho_dT_fd, 7e-10);
  }

  rho = _fp->rho(p, T);
  Real drho = 1e-4;

  {
    // internal energy
    Real de_dp_fd = (_fp->e(p + dp, rho) - _fp->e(p - dp, rho)) / (2 * dp);
    Real de_drho_fd = (_fp->e(p, rho + drho) - _fp->e(p, rho - drho)) / (2 * drho);
    Real de_dp = 0, de_drho = 0;
    _fp->e_dprho(p, rho, e, de_dp, de_drho);

    ABS_TEST("e", e, _fp->e(p, rho), 1e-16);
    ABS_TEST("de_dp", de_dp, de_dp_fd, 5e-11);
    ABS_TEST("de_drho", de_drho, de_drho_fd, 5e-6);
  }

  {
    // enthalpy
    Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2 * dp);
    Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2 * dT);

    Real h = 0, dh_dp = 0, dh_dT = 0;
    _fp->h_dpT(p, T, h, dh_dp, dh_dT);

    ABS_TEST("h", h, _fp->h(p, T), 1e-15);
    ABS_TEST("dh_dp", dh_dp, dh_dp_fd, 1e-15);
    ABS_TEST("dh_dT", dh_dT, dh_dT_fd, 2e-6);
  }

  // dp/dh and dp/ds for p(h,s)
  {
    Real h = 400.0;
    Real s = 1.0;

    Real dh = 1e-2 * h;
    Real ds = 1e-2 * s;

    Real dp_dh = _fp->dpdh_from_h_s(h, s);
    Real dp_ds = _fp->dpds_from_h_s(h, s);

    Real p_h_forward = _fp->p_from_h_s(h + dh, s);
    Real p_s_forward = _fp->p_from_h_s(h, s + ds);
    Real p_h_backward = _fp->p_from_h_s(h - dh, s);
    Real p_s_backward = _fp->p_from_h_s(h, s - ds);
    Real dp_dh_fd = (p_h_forward - p_h_backward) / (2 * dh);
    Real dp_ds_fd = (p_s_forward - p_s_backward) / (2 * ds);

    REL_TEST("dp_dh", dp_dh, dp_dh_fd, 1e-6);
    REL_TEST("dp_ds", dp_ds, dp_ds_fd, 1e-6);
  }

  // drho/dp, drho/ds, de/dp, and de/ds for rho(p,s) and e(p,s)
  {
    p = 1e6;
    T = 300;

    _fp->rho_e(p, T, rho, e);
    Real s = _fp->s(1 / rho, e);

    Real dp = 1e1;
    Real ds = 1e-4;

    Real rho, drho_dp, drho_ds, e, de_dp, de_ds;
    _fp->rho_e_dps(p, s, rho, drho_dp, drho_ds, e, de_dp, de_ds);

    Real rho1, rho2;
    Real e1, e2;

    _fp->rho_e_ps(p + dp, s, rho1, e1);
    _fp->rho_e_ps(p - dp, s, rho2, e2);
    Real drho_dp_fd = (rho1 - rho2) / (2 * dp);
    Real de_dp_fd = (e1 - e2) / (2 * dp);

    _fp->rho_e_ps(p, s + ds, rho1, e1);
    _fp->rho_e_ps(p, s - ds, rho2, e2);
    Real drho_ds_fd = (rho1 - rho2) / (2 * ds);
    Real de_ds_fd = (e1 - e2) / (2 * ds);

    REL_TEST("drho_dp", drho_dp, drho_dp_fd, 9e-8);
    REL_TEST("drho_ds", drho_ds, drho_ds_fd, 3e-8);
    REL_TEST("de_dp", de_dp, de_dp_fd, 2e-4);
    REL_TEST("de_ds", de_ds, de_ds_fd, 3e-8);
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
    REL_TEST("ds_dh", ds_dh, ds_dh_fd, rel_tol);
    REL_TEST("ds_dp", ds_dp, ds_dp_fd, rel_tol);
  }
}
