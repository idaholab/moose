//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidPropertiesPTTest.h"

/**
 * Verify calculation of the fluid properties
 */
TEST_F(IdealGasFluidPropertiesPTTest, properties)
{
  const Real molar_mass = 0.029;
  const Real thermal_expansion = 3.43e-3;
  const Real cv = 718.0;
  const Real cp = 1005.0;
  const Real thermal_conductivity = 0.02568;
  const Real entropy = 6850.0;
  const Real viscosity = 18.23e-6;
  const Real henry = 0.0;
  const Real R = 8.3144598;

  const Real tol = 1.0e-8;

  Real p, T;

  p = 1.0e6;
  T = 300.0;
  REL_TEST("beta", _fp->beta(p, T), thermal_expansion, tol);
  REL_TEST("cp", _fp->cp(p, T), cp, tol);
  REL_TEST("cv", _fp->cv(p, T), cv, tol);
  REL_TEST("c", _fp->c(p, T), std::sqrt(cp * R * T / (cv * molar_mass)), tol);
  REL_TEST("k", _fp->k(p, T), thermal_conductivity, tol);
  REL_TEST("k", _fp->k_from_rho_T(_fp->rho(p, T), T), thermal_conductivity, tol);
  REL_TEST("s", _fp->s(p, T), entropy, tol);
  REL_TEST("rho", _fp->rho(p, T), p * molar_mass / (R * T), tol);
  REL_TEST("e", _fp->e(p, T), cv * T, tol);
  REL_TEST("mu", _fp->mu(p, T), viscosity, tol);
  REL_TEST("mu", _fp->mu_from_rho_T(_fp->rho(p, T), T), viscosity, tol);
  REL_TEST("h", _fp->h(p, T), cp * T, tol);
  ABS_TEST("henry", _fp->henryConstant(T), henry, tol);
}

/**
 * Verify calculation of the derivatives by comparing with finite
 * differences
 */
TEST_F(IdealGasFluidPropertiesPTTest, derivatives)
{
  const Real dp = 1.0e1;
  const Real dT = 1.0e-4;

  const Real tol = 1.0e-8;

  Real fd;

  Real p = 1.0e6;
  Real T = 300.0;

  Real rho, drho_dp, drho_dT;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);
  fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
  REL_TEST("drho_dp", drho_dp, fd, tol);
  fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
  REL_TEST("drho_dT", drho_dT, fd, tol);

  Real e, de_dp, de_dT;
  _fp->e_dpT(p, T, e, de_dp, de_dT);
  ABS_TEST("de_dp", de_dp, 0.0, tol);
  fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
  REL_TEST("de_dT", de_dT, fd, tol);

  Real h, dh_dp, dh_dT;
  _fp->h_dpT(p, T, h, dh_dp, dh_dT);
  ABS_TEST("dh_dp", dh_dp, 0.0, tol);
  fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
  REL_TEST("dh_dT", dh_dT, fd, tol);

  Real mu, dmu_dp, dmu_dT;
  _fp->mu_dpT(p, T, mu, dmu_dp, dmu_dT);
  fd = (_fp->mu(p + dp, T) - _fp->mu(p - dp, T)) / (2.0 * dp);
  ABS_TEST("dmu_dp", dmu_dp, fd, tol);
  fd = (_fp->mu(p, T + dT) - _fp->mu(p, T - dT)) / (2.0 * dT);
  ABS_TEST("dh_dT", dmu_dT, fd, tol);
}
