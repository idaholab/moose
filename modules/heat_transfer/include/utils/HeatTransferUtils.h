//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

namespace HeatTransferUtils
{

/**
 * Compute Reynolds number
 *
 * @param rho  Density [kg/m^3]
 * @param vel  Velocity [m/s]
 * @param L  Characteristic length [m]
 * @param mu  Dynamic viscosity [Pa-s]
 * @return Reynolds number
 */
template <typename T1, typename T2, typename T3, typename T4>
auto
reynolds(const T1 & rho, const T2 & vel, const T3 & L, const T4 & mu)
{
  return rho * std::fabs(vel) * L / mu;
}

/**
 * Compute Prandtl number
 *
 * @param cp  Isobaric specific heat [J/K]
 * @param mu  Dynamic viscosity [Pa-s]
 * @param k  Thermal conductivity [W/m-K]
 * @return Prandtl number
 */
template <typename T1, typename T2, typename T3>
auto
prandtl(const T1 & cp, const T2 & mu, const T3 & k)
{
  return cp * mu / k;
}

/**
 * Compute Grashof number
 *
 * @param beta  Thermal expansion coefficient [1/K]
 * @param T_s  Surface temperature [K]
 * @param T_bulk  Bulk temperature [K]
 * @param L  Characteristic length [m]
 * @param rho  Density [kg/m^3]
 * @param mu  Dynamic viscosity [Pa-s]
 * @param gravity_magnitude  Gravitational acceleration magnitude
 * @return Grashof number
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
auto
grashof(const T1 & beta,
        const T2 & T_s,
        const T3 & T_bulk,
        const T4 & L,
        const T5 & rho,
        const T6 & mu,
        Real gravity_magnitude)
{
  return gravity_magnitude * beta * std::abs(T_s - T_bulk) * std::pow(L, 3) * (rho * rho) /
         (mu * mu);
}

/**
 * Compute Laplace number
 *
 * @param sigma  Surface tension [N/m]
 * @param rho  Density [kg/m^3]
 * @param L  Characteristic length [m]
 * @param mu  Dynamic viscosity [Pa-s]
 * @return Laplace number
 */
template <typename T1, typename T2, typename T3, typename T4>
auto
laplace(const T1 & sigma, const T2 & rho, const T3 & L, const T4 & mu)
{
  return sigma * rho * L / (mu * mu);
}

/**
 * Compute thermal diffusivity
 *
 * @param k Thermal conductivity [W/m-K]
 * @param rho Density [kg/m^3]
 * @param cp Isobaric specific heat [J/K]
 * @return Thermal diffusivity
 */
template <typename T1, typename T2, typename T3>
auto
thermalDiffusivity(const T1 & k, const T2 & rho, const T3 & cp)
{
  return k / (rho * cp);
}

/**
 * Compute Peclet number
 *
 * @param vel Velocity [m/s]
 * @param L  Characteristic length [m]
 * @param diffusivity Diffusivity
 * @return Peclet number
 */
template <typename T1, typename T2, typename T3>
auto
peclet(const T1 & vel, const T2 & L, const T3 & diffusivity)
{
  return std::fabs(vel) * L / diffusivity;
}

} // namespace HeatTransferUtils
