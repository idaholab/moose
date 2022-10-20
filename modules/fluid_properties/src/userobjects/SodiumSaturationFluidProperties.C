//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SodiumSaturationFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SodiumSaturationFluidProperties);

InputParameters
SodiumSaturationFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for liquid sodium at saturation conditions");
  return params;
}

SodiumSaturationFluidProperties::SodiumSaturationFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

std::string
SodiumSaturationFluidProperties::fluidName() const
{
  return "sodium_sat";
}

Real
SodiumSaturationFluidProperties::molarMass() const
{
  return 22.989769E-3;
}

Real
SodiumSaturationFluidProperties::rho_from_p_T(Real /* pressure */, Real temperature) const
{
  return 1.00423e3 - 0.21390 * temperature - 1.1046e-5 * temperature * temperature;
}

void
SodiumSaturationFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0.0;
  drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
}

void
SodiumSaturationFluidProperties::rho_from_p_T(const DualReal & pressure,
                                              const DualReal & temperature,
                                              DualReal & rho,
                                              DualReal & drho_dp,
                                              DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  drho_dp = 0.0;
  drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
}

Real
SodiumSaturationFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  return 1.0 / rho_from_p_T(pressure, temperature);
}

void
SodiumSaturationFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(pressure, temperature);
  dv_dp = 0.0;

  Real drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
  dv_dT = -v * v * drho_dT;
}

Real
SodiumSaturationFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  // h does not depend on pressure
  return (h_from_p_T(1e5, temperature) - e) / v;
}

Real
SodiumSaturationFluidProperties::T_from_v_e(Real v, Real /* e */) const
{
  // From inversion of second order polynomial form of rho(T)
  mooseAssert(0.2139 * 0.2139 + 4 * 1.1046e5 * (1.00423e3 - 1 / v) > 0,
              "Specific volume out of bounds");
  return (0.2139 - std::sqrt(0.2139 * 0.2139 + 4 * 1.1046e-5 * (1.00423e3 - 1 / v))) /
         (2 * -1.1046e-5);
}

Real
SodiumSaturationFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  Real t2 = temperature * temperature;
  return 3.7782E-10 * t2 * t2 * temperature / 5 - 1.7191E-6 * t2 * t2 / 4.0 +
         3.0921E-3 * t2 * temperature / 3.0 - 2.4560 * t2 / 2.0 + 1972.0 * temperature - 401088.7;
}

void
SodiumSaturationFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0.0;
  dh_dT = cp_from_p_T(pressure, temperature);
}

Real
SodiumSaturationFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  // definition of h = e + p * v
  Real v = v_from_p_T(pressure, temperature);
  Real h = h_from_p_T(pressure, temperature);
  return h - pressure * v;
}

void
SodiumSaturationFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(pressure, temperature);

  Real v, dv_dp, dv_dT;
  v_from_p_T(pressure, temperature, v, dv_dp, dv_dT);

  // definition of e = h - p * v, with dh/dp = 0
  de_dp = -pressure * dv_dp - v;

  // definition of e = h - p * v
  Real cp = cp_from_p_T(pressure, temperature);
  de_dT = cp - pressure * dv_dT;
}

Real
SodiumSaturationFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  Real t2 = temperature * temperature;
  return 3.7782E-10 * t2 * t2 - 1.7191E-6 * t2 * temperature + 3.0921E-3 * t2 -
         2.4560 * temperature + 1972.0;
}

void
SodiumSaturationFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0.0;

  Real t2 = temperature * temperature;
  dcp_dT =
      4 * 3.7782E-10 * t2 * temperature - 3 * 1.7191E-6 * t2 + 2 * 3.0921e-3 * temperature - 2.456;
}

Real
SodiumSaturationFluidProperties::cv_from_p_T(Real /* pressure */, Real temperature) const
{
  Real t2 = temperature * temperature;
  return 1.0369E-8 * temperature * t2 + 3.7164E-4 * t2 - 1.0494 * temperature + 1582.6;
}

void
SodiumSaturationFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 3 * 1.0369e-8 * temperature * temperature + 2 * 3.7164e-4 * temperature - 1.0494;
}

Real
SodiumSaturationFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 3.6522E-5 + 0.16626 / temperature - 4.56877e1 / (temperature * temperature) +
         2.8733E4 / (temperature * temperature * temperature);
}

void
SodiumSaturationFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;

  Real t2 = temperature * temperature;
  dmu_dT = 0.16626 * -1 / t2 - 4.56877E1 * -2 / (temperature * t2) + 2.8733E4 * -3 / (t2 * t2);
}

Real
SodiumSaturationFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 1.1045e2 - 6.5112e-2 * temperature + 1.5430e-5 * temperature * temperature -
         2.4617e-9 * temperature * temperature * temperature;
}

void
SodiumSaturationFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k_from_p_T(pressure, temperature);
  dk_dp = 0.0;
  dk_dT = -6.5112e-2 + 2 * 1.5430e-5 * temperature - 3 * 2.4617e-9 * temperature * temperature;
}
