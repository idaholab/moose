/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SimpleFluidPropertiesTest.h"

/**
 * Verify calculation of the fluid properties
 */
TEST_F(SimpleFluidPropertiesTest, properties)
{
  const Real thermal_exp = 2.14E-4;
  const Real cv = 4186.0;
  const Real cp = 4194.0;
  const Real bulk_modulus = 2.0E9;
  const Real thermal_cond = 0.6;
  const Real entropy = 300.0;
  const Real visc = 1.0E-3;
  const Real density0 = 1000.0;
  const Real henry = 0.0;

  Real P, T;

  P = 1E3;
  T = 200;
  REL_TEST("beta", _fp->beta(P, T), thermal_exp, 1.0E-8);
  REL_TEST("cp", _fp->cp(P, T), cp, 1.0E-8);
  REL_TEST("cv", _fp->cv(P, T), cv, 1.0E-8);
  REL_TEST("c", _fp->c(P, T), std::sqrt(bulk_modulus / _fp->rho(P, T)), 1.0E-8);
  REL_TEST("k", _fp->k(P, T), thermal_cond, 1.0E-8);
  REL_TEST("s", _fp->s(P, T), entropy, 1.0E-8);
  REL_TEST("rho", _fp->rho(P, T), density0 * std::exp(P / bulk_modulus - thermal_exp * T), 1.0E-8);
  REL_TEST("e", _fp->e(P, T), cv * T, 1.0E-8);
  REL_TEST("mu", _fp->mu(P, T), visc, 1.0E-8);
  REL_TEST("h", _fp->h(P, T), cp * T, 1.0E-8);
  ABS_TEST("henry", _fp->henryConstant(T), henry, 1.0E-8);

  P = 1E7;
  T = 300;
  REL_TEST("beta", _fp->beta(P, T), thermal_exp, 1.0E-8);
  REL_TEST("cp", _fp->cp(P, T), cp, 1.0E-8);
  REL_TEST("cv", _fp->cv(P, T), cv, 1.0E-8);
  REL_TEST("c", _fp->c(P, T), std::sqrt(bulk_modulus / _fp->rho(P, T)), 1.0E-8);
  REL_TEST("k", _fp->k(P, T), thermal_cond, 1.0E-8);
  REL_TEST("s", _fp->s(P, T), entropy, 1.0E-8);
  REL_TEST("rho", _fp->rho(P, T), density0 * std::exp(P / bulk_modulus - thermal_exp * T), 1.0E-8);
  REL_TEST("e", _fp->e(P, T), cv * T, 1.0E-8);
  REL_TEST("mu", _fp->mu(P, T), visc, 1.0E-8);
  REL_TEST("h", _fp->h(P, T), cp * T, 1.0E-8);
  ABS_TEST("henry", _fp->henryConstant(T), henry, 1.0E-8);
}

/**
 * Verify calculation of the derivatives by comparing with finite
 * differences
 */
TEST_F(SimpleFluidPropertiesTest, derivatives)
{
  const Real dP = 1.0E1;
  const Real dT = 1.0E-4;
  const Real ddens = 1.0E-3;

  Real fd;
  Real P, T, dens;

  Real rho, drho_dp, drho_dT;
  P = 1E7;
  T = 10;
  _fp->rho_dpT(P, T, rho, drho_dp, drho_dT);
  fd = (_fp->rho(P + dP, T) - _fp->rho(P - dP, T)) / (2.0 * dP);
  REL_TEST("drho_dP", drho_dp, fd, 1.0E-8);
  fd = (_fp->rho(P, T + dT) - _fp->rho(P, T - dT)) / (2.0 * dT);
  REL_TEST("drho_dT", drho_dT, fd, 1.0E-8);

  P = 5E7;
  T = 90;
  _fp->rho_dpT(P, T, rho, drho_dp, drho_dT);
  fd = (_fp->rho(P + dP, T) - _fp->rho(P - dP, T)) / (2.0 * dP);
  REL_TEST("drho_dP", drho_dp, fd, 1.0E-8);
  fd = (_fp->rho(P, T + dT) - _fp->rho(P, T - dT)) / (2.0 * dT);
  REL_TEST("drho_dT", drho_dT, fd, 1.0E-8);

  Real e, de_dp, de_dT;
  P = 1E6;
  T = 20;
  _fp->e_dpT(P, T, e, de_dp, de_dT);
  fd = (_fp->e(P + dP, T) - _fp->e(P - dP, T)) / (2.0 * dP);
  ABS_TEST("de_dP", de_dp, fd, 1.0E-8);
  fd = (_fp->e(P, T + dT) - _fp->e(P, T - dT)) / (2.0 * dT);
  REL_TEST("de_dT", de_dT, fd, 1.0E-8);

  P = 5E6;
  T = 90;
  _fp->e_dpT(P, T, e, de_dp, de_dT);
  fd = (_fp->e(P + dP, T) - _fp->e(P - dP, T)) / (2.0 * dP);
  ABS_TEST("de_dP", de_dp, fd, 1.0E-8);
  fd = (_fp->e(P, T + dT) - _fp->e(P, T - dT)) / (2.0 * dT);
  REL_TEST("de_dT", de_dT, fd, 1.0E-8);

  Real mu, dmu_drho, dmu_dT;
  dens = 1000;
  T = 10;
  _fp->mu_drhoT(dens, T, mu, dmu_drho, dmu_dT);
  fd = (_fp->mu(dens + ddens, T) - _fp->mu(dens - ddens, T)) / (2.0 * ddens);
  ABS_TEST("dmu_ddens", dmu_drho, fd, 1.0E-8);
  fd = (_fp->mu(dens, T + dT) - _fp->mu(dens, T - dT)) / (2.0 * dT);
  ABS_TEST("dmu_dT", dmu_dT, fd, 1.0E-8);

  dens = 2000;
  T = 80;
  _fp->mu_drhoT(dens, T, mu, dmu_drho, dmu_dT);
  fd = (_fp->mu(dens + ddens, T) - _fp->mu(dens - ddens, T)) / (2.0 * ddens);
  ABS_TEST("dmu_ddens", dmu_drho, fd, 1.0E-8);
  fd = (_fp->mu(dens, T + dT) - _fp->mu(dens, T - dT)) / (2.0 * dT);
  ABS_TEST("dmu_dT", dmu_dT, fd, 1.0E-8);

  Real h, dh_dp, dh_dT;
  P = 1E6;
  T = 10;
  _fp->h_dpT(P, T, h, dh_dp, dh_dT);
  fd = (_fp->h(P + dP, T) - _fp->h(P - dP, T)) / (2.0 * dP);
  ABS_TEST("dh_dP", dh_dp, fd, 1.0E-8);
  fd = (_fp->h(P, T + dT) - _fp->h(P, T - dT)) / (2.0 * dT);
  REL_TEST("dh_dT", dh_dT, fd, 1.0E-8);

  P = 4E6;
  T = 90;
  _fp->h_dpT(P, T, h, dh_dp, dh_dT);
  fd = (_fp->h(P + dP, T) - _fp->h(P - dP, T)) / (2.0 * dP);
  ABS_TEST("dh_dP", dh_dp, fd, 1.0E-8);
  fd = (_fp->h(P, T + dT) - _fp->h(P, T - dT)) / (2.0 * dT);
  REL_TEST("dh_dT", dh_dT, fd, 1.0E-8);
}
