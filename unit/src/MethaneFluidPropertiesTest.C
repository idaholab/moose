//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MethaneFluidPropertiesTest.h"

/**
 * Verify calculation of Henry's constant using data from
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(MethaneFluidPropertiesTest, henry)
{
  REL_TEST("henry", _fp->henryConstant(300.0), 4069.0e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(400.0), 6017.1e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(500.0), 2812.9e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(600.0), 801.8e6, 1.0e-3);
}

/**
 * Verify calculation of thermophysical properties of methane using
 * verification data provided in
 * Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
 * Computer Equations
 */
TEST_F(MethaneFluidPropertiesTest, properties)
{
  // Pressure = 10 MPa, temperature = 350 K
  Real p = 10.0e6;
  Real T = 350.0;

  REL_TEST("density", _fp->rho(p, T), 55.13, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), 708.5e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), 527.131e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), 11.30e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 2.375e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 1.857e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 481.7, 1.0e-3);
  REL_TEST("mu", _fp->mu_from_rho_T(0.0, T), 0.01276e-3, 1.0e-3);
  REL_TEST("thermal conductivity", _fp->k_from_rho_T(0.0, T), 0.04113, 1.0e-3);
}

/**
 * Verify calculation of the derivatives of all properties by comparing with finite
 * differences
 */
TEST_F(MethaneFluidPropertiesTest, derivatives)
{
  Real p = 10.0e6;
  Real T = 350.0;

  // Finite differencing parameters
  Real dp = 1.0e1;
  Real dT = 1.0e-4;

  // Density
  Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
  Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

  ABS_TEST("rho", rho, _fp->rho(p, T), 1.0e-15);
  REL_TEST("drho_dp", drho_dp, drho_dp_fd, 1.0e-6);
  REL_TEST("drho_dT", drho_dT, drho_dT_fd, 1.0e-6);

  // Enthalpy
  Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0;
  _fp->h_dpT(p, T, h, dh_dp, dh_dT);

  ABS_TEST("h", h, _fp->h(p, T), 1.0e-15);
  ABS_TEST("dh_dp", dh_dp, dh_dp_fd, 1.0e-15);
  REL_TEST("dh_dT", dh_dT, dh_dT_fd, 1.0e-6);

  // Internal energy
  Real de_dp_fd = (_fp->e(p + dp, T) - _fp->e(p - dp, T)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
  Real e = 0.0, de_dp = 0.0, de_dT = 0.0;
  _fp->e_dpT(p, T, e, de_dp, de_dT);

  ABS_TEST("e", e, _fp->e(p, T), 1.0e-15);
  ABS_TEST("de_dp", de_dp, de_dp_fd, 1.0e-15);
  REL_TEST("de_dT", de_dT, de_dT_fd, 1.0e-6);

  // Viscosity
  rho = 1.0; // Not used in correlations
  Real drho = 1.0e-4;

  Real dmu_drho_fd =
      (_fp->mu_from_rho_T(rho + drho, T) - _fp->mu_from_rho_T(rho - drho, T)) / (2.0 * drho);
  Real dmu_dT_fd = (_fp->mu_from_rho_T(rho, T + dT) - _fp->mu_from_rho_T(rho, T - dT)) / (2.0 * dT);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_drhoT_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu_from_rho_T(rho, T), 1.0e-15);
  ABS_TEST("dmu_drho", dmu_drho, dmu_drho_fd, 1.0e-15);
  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);

  Real dmu_dp_fd = (_fp->mu(p + dp, T) - _fp->mu(p - dp, T)) / (2.0 * dp);
  dmu_dT_fd = (_fp->mu(p, T + dT) - _fp->mu(p, T - dT)) / (2.0 * dT);
  Real dmu_dp = 0.0;
  _fp->mu_dpT(p, T, mu, dmu_dp, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu(p, T), 1.0e-15);
  ABS_TEST("dmu_dp", dmu_dp, dmu_dp_fd, 1.0e-15);
  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);

  // Henry's constant
  T = 300.0;

  Real dKh_dT_fd = (_fp->henryConstant(T + dT) - _fp->henryConstant(T - dT)) / (2.0 * dT);
  Real Kh = 0.0, dKh_dT = 0.0;
  _fp->henryConstant_dT(T, Kh, dKh_dT);
  REL_TEST("henry", Kh, _fp->henryConstant(T), 1.0e-6);
  REL_TEST("dhenry_dT", dKh_dT_fd, dKh_dT, 1.0e-6);
}
