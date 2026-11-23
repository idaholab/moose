//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LithiumFluidProperties.h"

registerMooseObject("FluidPropertiesApp", LithiumFluidProperties);

InputParameters
LithiumFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for liquid lithium");
  // The base class default (400 K) is below the melting point, outside the correlation range
  params.set<Real>("T_initial_guess") = 800;
  return params;
}

LithiumFluidProperties::LithiumFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

std::string
LithiumFluidProperties::fluidName() const
{
  return "lithium";
}

Real
LithiumFluidProperties::molarMass() const
{
  return _M;
}

void
LithiumFluidProperties::checkTemperature(Real temperature, const std::string & property) const
{
  if (temperature < _T_min || temperature > _T_max)
    flagInvalidSolution("Lithium " + property +
                        " evaluated outside of the correlation temperature range [455, 1500] K");
}

Real
LithiumFluidProperties::cpIntegral(Real temperature)
{
  const Real T = temperature;
  return 4754. * T - 0.925 / 2 * T * T + 2.91e-4 / 3 * T * T * T;
}

Real
LithiumFluidProperties::T_from_p_h(Real pressure, Real enthalpy) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, current_T, new_h, dh_dp, dh_dT); };
  const Real T = FluidPropertiesUtils::NewtonSolve(pressure,
                                                   enthalpy,
                                                   _T_initial_guess,
                                                   _tolerance,
                                                   lambda,
                                                   name() + "::T_from_p_h",
                                                   _max_newton_its)
                     .first;
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               pressure,
               ") and enthalpy (h = ",
               enthalpy,
               ") to temperature failed to converge.");
  return T;
}

Real
LithiumFluidProperties::T_from_p_rho(Real pressure, Real density) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_rho, Real & drho_dp, Real & drho_dT)
  { rho_from_p_T(p, current_T, new_rho, drho_dp, drho_dT); };
  const Real T = FluidPropertiesUtils::NewtonSolve(pressure,
                                                   density,
                                                   _T_initial_guess,
                                                   _tolerance,
                                                   lambda,
                                                   name() + "::T_from_p_rho",
                                                   _max_newton_its)
                     .first;
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               pressure,
               ") and density (rho = ",
               density,
               ") to temperature failed to converge.");
  return T;
}

Real
LithiumFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "density");
  return 278.5 - 0.04657 * temperature + 274.6 * std::pow(1 - temperature / 3500., 0.467);
}

void
LithiumFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  drho_dT = -0.04657 - 274.6 * 0.467 / 3500. * std::pow(1 - temperature / 3500., 0.467 - 1);
}

Real
LithiumFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  return h_from_p_T(pressure, temperature) - pressure / rho_from_p_T(pressure, temperature);
}

void
LithiumFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  Real h, dh_dp, dh_dT;
  h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  e = h - pressure / rho;
  de_dp = dh_dp + pressure * drho_dp / rho / rho - 1.0 / rho;
  de_dT = dh_dT + pressure * drho_dT / rho / rho;
}

Real
LithiumFluidProperties::e_from_p_rho(Real pressure, Real density) const
{
  return e_from_p_T(pressure, T_from_p_rho(pressure, density));
}

Real
LithiumFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "specific heat");
  return 4754. - 0.925 * temperature + 2.91e-4 * temperature * temperature;
}

void
LithiumFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  dcp_dT = -0.925 + 2 * 2.91e-4 * temperature;
}

Real
LithiumFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  // The density correlation does not depend on pressure, so the fluid is incompressible and,
  // as is conventional for an incompressible liquid, cv = cp
  return cp_from_p_T(pressure, temperature);
}

Real
LithiumFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "viscosity");
  return std::exp(-4.164 - 0.6374 * std::log(temperature) + 292.1 / temperature);
}

void
LithiumFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(pressure, temperature);
  dmu_dp = 0;
  dmu_dT = mu * (-0.6374 / temperature - 292.1 / temperature / temperature);
}

Real
LithiumFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "thermal conductivity");
  return 22.28 + 0.0500 * temperature - 1.243e-5 * temperature * temperature;
}

void
LithiumFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(pressure, temperature);
  dk_dp = 0;
  dk_dT = 0.0500 - 2 * 1.243e-5 * temperature;
}

Real
LithiumFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "enthalpy");
  // Integral of the specific heat, relative to the liquid at the melting temperature
  return cpIntegral(temperature) - cpIntegral(_T_melt);
}

void
LithiumFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0;
  dh_dT = cp_from_p_T(pressure, temperature);
}
