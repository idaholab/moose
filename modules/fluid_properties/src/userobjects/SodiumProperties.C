//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SodiumProperties.h"
#include "MooseError.h"

template <>
InputParameters
validParams<SodiumProperties>()
{
  InputParameters params = validParams<FluidProperties>();
  params.addClassDescription("Fluid properties for sodium");
  return params;
}

SodiumProperties::SodiumProperties(const InputParameters & parameters) : FluidProperties(parameters)
{
}

Real
SodiumProperties::k(Real T) const
{
  const Real T2 = T * T;
  const Real T3 = T2 * T;
  return 124.67 - 0.11381 * T + 5.5226e-5 * T2 - 1.1842e-8 * T3;
}

Real
SodiumProperties::h(Real T) const
{
  const Real T2 = T * T;
  const Real T3 = T2 * T;

  // Converted from kJ/kg to J/kg.
  return -365.77e3 + 1.6582e3 * T - 4.2395e-1 * T2 + 1.4847e-4 * T3 + 2992.6e3 / T;
}

Real
SodiumProperties::heatCapacity(Real T) const
{
  const Real T2 = T * T;
  // Converted from kJ/kg-K to J/kg-K.
  return 1.6582e3 - 8.4790e-1 * T + 4.4541e-4 * T2 - 2992.6e3 / T2;
}

Real
SodiumProperties::temperature(Real H) const
{
  // Estimate initial guess from linear part of enthalpy.
  Real T = (H + 365.77e3) / 1.6582e3;

  // Newton-Raphson for this equation: enthalpy(T) - H = 0 = residual. This is easy because
  // dResidual/dT is just dH/dT, which is heat capacity.
  for (unsigned iteration = 0; iteration < 10; ++iteration)
  {
    Real residual = h(T) - H;
    T -= residual / heatCapacity(T);
    if (std::abs(residual / H) < 1e-6)
      break;
  }
  // If we get here, enthalpy is probably out of bounds. However, due to the nature of the JFNK
  // calculation, we probably just want to ignore the error and spit out a bogus T so that the
  // solver keeps rolling.
  return T;
}

Real
SodiumProperties::rho(Real T) const
{
  const Real rhoc = 219.0; // kg/m^3
  const Real f = 275.32;
  const Real g = 511.58;
  const Real Tc = 2503.7; // critical temperature, K
  mooseAssert(T < Tc, "Temperature is greater than critical temperature 2503.7 K ");

  return rhoc + f * (1 - T / Tc) + g * std::sqrt(1 - T / Tc);
}

Real
SodiumProperties::drho_dT(Real T) const
{
  const Real f = 275.32;
  const Real g = 511.58;
  const Real Tc = 2503.7; // critical temperature, K
  mooseAssert(T < Tc, "Temperature is greater than critical temperature 2503.7 K ");

  return -(f + g * (0.5) / std::sqrt(1 - T / Tc)) / Tc;
}

Real
SodiumProperties::drho_dh(Real h) const
{
  Real T = 0.0;
  T = temperature(h);
  return drho_dT(T) / heatCapacity(T);
}
