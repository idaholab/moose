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
  if (temperature < _T_melt || temperature > _T_max)
    flagInvalidSolution("Lithium " + property +
                        " evaluated outside of the correlation temperature range [453.7, 1600] K");
}

Real
LithiumFluidProperties::T_from_p_h(Real /*pressure*/, Real enthalpy) const
{
  // Inversion of eq. 3
  return (enthalpy + 7.519e5) / _cp;
}

Real
LithiumFluidProperties::T_from_p_rho(Real /*pressure*/, Real density) const
{
  // Inversion of eq. 1
  return (562. - density) / 0.1;
}

Real
LithiumFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "density");
  // Eq. 1
  return 562. - 0.1 * temperature;
}

void
LithiumFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  drho_dT = -0.1;
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
  // Eq. 5, derivative of the linear enthalpy correlation (eq. 3)
  return _cp;
}

void
LithiumFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  dcp_dT = 0;
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
  // Eq. 10, in N.s/m^2
  return std::pow(10., -3.080 + 57.63 / temperature - 5.172e-4 * temperature);
}

void
LithiumFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(pressure, temperature);
  dmu_dp = 0;
  dmu_dT = mu * std::log(10.) * (-57.63 / temperature / temperature - 5.172e-4);
}

Real
LithiumFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "thermal conductivity");
  // Polynomial fit of the modified Ewing correlation (eq. 8), page 11
  return 21.874 + 0.056255 * temperature - 1.8325e-5 * temperature * temperature;
}

void
LithiumFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(pressure, temperature);
  dk_dp = 0;
  dk_dT = 0.056255 - 2 * 1.8325e-5 * temperature;
}

Real
LithiumFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  checkTemperature(temperature, "enthalpy");
  // Eq. 3, relative to solid lithium at 273 K
  return -7.519e5 + _cp * temperature;
}

void
LithiumFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0;
  dh_dT = _cp;
}
