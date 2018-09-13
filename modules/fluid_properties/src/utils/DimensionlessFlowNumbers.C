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

Real
prandtl(Real cp, Real mu, Real k)
{
  return cp * mu / k;
}

Real
grashof(Real beta, Real T_s, Real T_bulk, Real L, Real rho, Real mu, Real gravity_magnitude)
{
  return gravity_magnitude * beta * std::abs(T_s - T_bulk) * std::pow(L, 3) * (rho * rho) /
         (mu * mu);
}

Real
laplace(Real sigma, Real rho, Real L, Real mu)
{
  return sigma * rho * L / (mu * mu);
}

Real
thermalDiffusivity(Real k, Real rho, Real cp)
{
  return k / (rho * cp);
}

Real
peclet(Real vel, Real L, Real diffusivity)
{
  return std::fabs(vel) * L / diffusivity;
}

} // namespace fp
