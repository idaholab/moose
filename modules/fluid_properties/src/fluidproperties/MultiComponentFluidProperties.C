//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiComponentFluidProperties.h"

InputParameters
MultiComponentFluidProperties::validParams()
{
  InputParameters params = FluidProperties::validParams();
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
  mooseError("fluidName() is not implemented");
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

const SinglePhaseFluidProperties &
MultiComponentFluidProperties::getComponent(unsigned int) const
{
  mooseError("getComponent() is not implemented");
}
