//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DimensionlessFlowNumbers.h"

#include "DualRealOps.h"

namespace fp
{

Real
reynolds(Real rho, Real vel, Real L, Real mu)
{
  return rho * std::fabs(vel) * L / mu;
}

DualReal
reynolds(DualReal rho, DualReal vel, DualReal L, DualReal mu)
{
  return rho * std::fabs(vel) * L / mu;
}

Real
prandtl(Real cp, Real mu, Real k)
{
  return cp * mu / k;
}

DualReal
prandtl(DualReal cp, DualReal mu, DualReal k)
{
  return cp * mu / k;
}

Real
grashof(Real beta, Real T_s, Real T_bulk, Real L, Real rho, Real mu, Real gravity_magnitude)
{
  return gravity_magnitude * beta * std::abs(T_s - T_bulk) * std::pow(L, 3) * (rho * rho) /
         (mu * mu);
}

DualReal
grashof(DualReal beta,
        DualReal T_s,
        DualReal T_bulk,
        DualReal L,
        DualReal rho,
        DualReal mu,
        DualReal gravity_magnitude)
{
  return gravity_magnitude * beta * std::abs(T_s - T_bulk) * std::pow(L, 3) * (rho * rho) /
         (mu * mu);
}

Real
laplace(Real sigma, Real rho, Real L, Real mu)
{
  return sigma * rho * L / (mu * mu);
}

DualReal
laplace(DualReal sigma, DualReal rho, DualReal L, DualReal mu)
{
  return sigma * rho * L / (mu * mu);
}

Real
thermalDiffusivity(Real k, Real rho, Real cp)
{
  return k / (rho * cp);
}

DualReal
thermalDiffusivity(DualReal k, DualReal rho, DualReal cp)
{
  return k / (rho * cp);
}

Real
peclet(Real vel, Real L, Real diffusivity)
{
  return std::fabs(vel) * L / diffusivity;
}

DualReal
peclet(DualReal vel, DualReal L, DualReal diffusivity)
{
  return std::fabs(vel) * L / diffusivity;
}

} // namespace fp
