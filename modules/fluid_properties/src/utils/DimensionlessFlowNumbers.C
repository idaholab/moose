//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DimensionlessFlowNumbers.h"

namespace fp
{

Real
reynolds(Real rho, Real vel, Real L, Real mu)
{
  return rho * std::fabs(vel) * L / mu;
}

ADReal
reynolds(const ADReal & rho, const ADReal & vel, const ADReal & L, const ADReal & mu)
{
  return rho * std::fabs(vel) * L / mu;
}

Real
prandtl(Real cp, Real mu, Real k)
{
  return cp * mu / k;
}

ADReal
prandtl(const ADReal & cp, const ADReal & mu, const ADReal & k)
{
  return cp * mu / k;
}

Real
grashof(Real beta, Real T_s, Real T_bulk, Real L, Real rho, Real mu, Real gravity_magnitude)
{
  return gravity_magnitude * beta * std::abs(T_s - T_bulk) * std::pow(L, 3) * (rho * rho) /
         (mu * mu);
}

ADReal
grashof(const ADReal & beta,
        const ADReal & T_s,
        const ADReal & T_bulk,
        const ADReal & L,
        const ADReal & rho,
        const ADReal & mu,
        const ADReal & gravity_magnitude)
{
  return gravity_magnitude * beta * std::abs(T_s - T_bulk) * std::pow(L, 3) * (rho * rho) /
         (mu * mu);
}

Real
laplace(Real sigma, Real rho, Real L, Real mu)
{
  return sigma * rho * L / (mu * mu);
}

ADReal
laplace(const ADReal & sigma, const ADReal & rho, const ADReal & L, const ADReal & mu)
{
  return sigma * rho * L / (mu * mu);
}

Real
thermalDiffusivity(Real k, Real rho, Real cp)
{
  return k / (rho * cp);
}

ADReal
thermalDiffusivity(const ADReal & k, const ADReal & rho, const ADReal & cp)
{
  return k / (rho * cp);
}

Real
peclet(Real vel, Real L, Real diffusivity)
{
  return std::fabs(vel) * L / diffusivity;
}

ADReal
peclet(const ADReal & vel, const ADReal & L, const ADReal & diffusivity)
{
  return std::fabs(vel) * L / diffusivity;
}

} // namespace fp
