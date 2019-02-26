//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiComponentFluidProperties.h"

template <>
InputParameters
validParams<MultiComponentFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();
  return params;
}

MultiComponentFluidProperties::MultiComponentFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

MultiComponentFluidProperties::~MultiComponentFluidProperties() {}

std::string
MultiComponentFluidProperties::fluidName() const
{
  mooseError(name(), ": fluidName() is not implemented");
}

Real
MultiComponentFluidProperties::rho(Real pressure, Real temperature, Real xmass) const
{
  mooseDeprecated(name(), ": rho() is deprecated. Use rho_from_p_T_X() instead");
  return rho_from_p_T_X(pressure, temperature, xmass);
}

void
MultiComponentFluidProperties::rho_dpTx(Real pressure,
                                        Real temperature,
                                        Real xmass,
                                        Real & rho,
                                        Real & drho_dp,
                                        Real & drho_dT,
                                        Real & drho_dx) const
{
  mooseDeprecated(name(), ": rho_dpTx() is deprecated. Use rho_from_p_T_X() instead");
  rho_from_p_T_X(pressure, temperature, xmass, rho, drho_dp, drho_dT, drho_dx);
}

Real
MultiComponentFluidProperties::mu(Real pressure, Real temperature, Real xmass) const
{
  mooseDeprecated(name(), ": mu() is deprecated. Use mu_from_p_T_X() instead");
  return mu_from_p_T_X(pressure, temperature, xmass);
}

void
MultiComponentFluidProperties::mu_dpTx(Real pressure,
                                       Real temperature,
                                       Real xmass,
                                       Real & mu,
                                       Real & dmu_dp,
                                       Real & dmu_dT,
                                       Real & dmu_dx) const
{
  mooseDeprecated(name(), ": mu_dpTx() is deprecated. Use mu_from_p_T_X() instead");
  mu_from_p_T_X(pressure, temperature, xmass, mu, dmu_dp, dmu_dT, dmu_dx);
}

void
MultiComponentFluidProperties::rho_mu_from_p_T_X(
    Real pressure, Real temperature, Real xmass, Real & rho, Real & mu) const
{
  rho = rho_from_p_T_X(pressure, temperature, xmass);
  mu = mu_from_p_T_X(pressure, temperature, xmass);
}

void
MultiComponentFluidProperties::rho_mu_from_p_T_X(
    DualReal pressure, DualReal temperature, DualReal xmass, DualReal & rho, DualReal & mu) const
{
  rho = rho_from_p_T_X(pressure, temperature, xmass);
  mu = mu_from_p_T_X(pressure, temperature, xmass);
}

void
MultiComponentFluidProperties::rho_mu(
    Real pressure, Real temperature, Real xmass, Real & rho, Real & mu) const
{
  mooseDeprecated(name(), ": rho_mu() is deprecated. Use rho_mu_from_p_T_X() instead");
  rho_mu_from_p_T_X(pressure, temperature, xmass, rho, mu);
}

void
MultiComponentFluidProperties::rho_mu_from_p_T_X(Real pressure,
                                                 Real temperature,
                                                 Real xmass,
                                                 Real & rho,
                                                 Real & drho_dp,
                                                 Real & drho_dT,
                                                 Real & drho_dx,
                                                 Real & mu,
                                                 Real & dmu_dp,
                                                 Real & dmu_dT,
                                                 Real & dmu_dx) const
{
  rho_from_p_T_X(pressure, temperature, xmass, rho, drho_dp, drho_dT, drho_dx);
  mu_from_p_T_X(pressure, temperature, xmass, mu, dmu_dp, dmu_dT, dmu_dx);
}

void
MultiComponentFluidProperties::rho_mu_dpTx(Real pressure,
                                           Real temperature,
                                           Real xmass,
                                           Real & rho,
                                           Real & drho_dp,
                                           Real & drho_dT,
                                           Real & drho_dx,
                                           Real & mu,
                                           Real & dmu_dp,
                                           Real & dmu_dT,
                                           Real & dmu_dx) const
{
  mooseDeprecated(name(), ": rho_mu_dpTx() is deprecated. Use rho_mu_from_p_T_X() instead");
  rho_mu_from_p_T_X(
      pressure, temperature, xmass, rho, drho_dp, drho_dT, drho_dx, mu, dmu_dp, dmu_dT, dmu_dx);
}

Real
MultiComponentFluidProperties::h(Real pressure, Real temperature, Real xmass) const
{
  mooseDeprecated(name(), ": h() is deprecated. Use h_from_p_T_X() instead");
  return h_from_p_T_X(pressure, temperature, xmass);
}

void
MultiComponentFluidProperties::h_dpTx(Real pressure,
                                      Real temperature,
                                      Real xmass,
                                      Real & h,
                                      Real & dh_dp,
                                      Real & dh_dT,
                                      Real & dh_dx) const
{
  mooseDeprecated(name(), ": h_dpTx() is deprecated. Use h_from_p_T_X() instead");
  h_from_p_T_X(pressure, temperature, xmass, h, dh_dp, dh_dT, dh_dx);
}

Real
MultiComponentFluidProperties::cp(Real pressure, Real temperature, Real xmass) const
{
  mooseDeprecated(name(), ": cp() is deprecated. Use cp_from_p_T_X() instead");
  return cp_from_p_T_X(pressure, temperature, xmass);
}

Real
MultiComponentFluidProperties::e(Real pressure, Real temperature, Real xmass) const
{
  mooseDeprecated(name(), ": e() is deprecated. Use e_from_p_T_X() instead");
  return e_from_p_T_X(pressure, temperature, xmass);
}

void
MultiComponentFluidProperties::e_dpTx(Real pressure,
                                      Real temperature,
                                      Real xmass,
                                      Real & e,
                                      Real & de_dp,
                                      Real & de_dT,
                                      Real & de_dx) const
{
  mooseDeprecated(name(), ": e_dpTx() is deprecated. Use e_from_p_T_X() instead");
  e_from_p_T_X(pressure, temperature, xmass, e, de_dp, de_dT, de_dx);
}

Real
MultiComponentFluidProperties::k(Real pressure, Real temperature, Real xmass) const
{
  mooseDeprecated(name(), ": k() is deprecated. Use k_from_p_T_X() instead");
  return k_from_p_T_X(pressure, temperature, xmass);
}

const SinglePhaseFluidProperties &
MultiComponentFluidProperties::getComponent(unsigned int) const
{
  mooseError(name(), ": getComponent() is not implemented");
}
