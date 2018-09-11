//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SimpleFluidProperties);

template <>
InputParameters
validParams<SimpleFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addParam<Real>("molar_mass", 1.8E-2, "Constant molar mass of the fluid (kg/mol)");
  params.addParam<Real>(
      "thermal_expansion", 2.14E-4, "Constant coefficient of thermal expansion (1/K)");
  params.addParam<Real>(
      "cv", 4186.0, "Constant specific heat capacity at constant volume (J/kg/K)");
  params.addParam<Real>(
      "cp", 4194.0, "Constant specific heat capacity at constant pressure (J/kg/K)");
  params.addRangeCheckedParam<Real>(
      "bulk_modulus", 2.0E9, "bulk_modulus>0", "Constant bulk modulus (Pa)");
  params.addParam<Real>("thermal_conductivity", 0.6, "Constant thermal conductivity (W/m/K)");
  params.addParam<Real>("specific_entropy", 300.0, "Constant specific entropy (J/kg/K)");
  params.addParam<Real>("viscosity", 1.0E-3, "Constant dynamic viscosity (Pa.s)");
  params.addParam<Real>("density0", 1000.0, "Density at zero pressure and zero temperature");
  params.addParam<Real>("henry_constant", 0.0, "Henry constant for dissolution in water");
  params.addParam<Real>("porepressure_coefficient",
                        1.0,
                        "The enthalpy is internal_energy + P / density * "
                        "porepressure_coefficient.  Physically this should be 1.0, "
                        "but analytic solutions are simplified when it is zero");
  params.addClassDescription("Fluid properties for a simple fluid with a constant bulk density");
  return params;
}

SimpleFluidProperties::SimpleFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters),
    _molar_mass(getParam<Real>("molar_mass")),
    _thermal_expansion(getParam<Real>("thermal_expansion")),
    _cv(getParam<Real>("cv")),
    _cp(getParam<Real>("cp")),
    _bulk_modulus(getParam<Real>("bulk_modulus")),
    _thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _specific_entropy(getParam<Real>("specific_entropy")),
    _viscosity(getParam<Real>("viscosity")),
    _density0(getParam<Real>("density0")),
    _henry_constant(getParam<Real>("henry_constant")),
    _pp_coeff(getParam<Real>("porepressure_coefficient"))
{
}

SimpleFluidProperties::~SimpleFluidProperties() {}

std::string
SimpleFluidProperties::fluidName() const
{
  return "simple_fluid";
}

Real
SimpleFluidProperties::molarMass() const
{
  return _molar_mass;
}

Real SimpleFluidProperties::beta_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _thermal_expansion;
}

Real SimpleFluidProperties::cp_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cp;
}

Real SimpleFluidProperties::cv_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cv;
}

Real
SimpleFluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  return std::sqrt(_bulk_modulus / rho_from_p_T(pressure, temperature));
}

Real SimpleFluidProperties::k_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _thermal_conductivity;
}

void
SimpleFluidProperties::k_from_p_T(
    Real /*pressure*/, Real /*temperature*/, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = _thermal_conductivity;
  dk_dp = 0;
  dk_dT = 0;
}

Real SimpleFluidProperties::s_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _specific_entropy;
}

void
SimpleFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  SinglePhaseFluidProperties::s_from_p_T(p, T, s, ds_dp, ds_dT);
}

Real
SimpleFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  return _density0 * std::exp(pressure / _bulk_modulus - _thermal_expansion * temperature);
}

void
SimpleFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  drho_dp = rho / _bulk_modulus;
  drho_dT = -_thermal_expansion * rho;
}

Real
SimpleFluidProperties::e_from_p_T(Real /*pressure*/, Real temperature) const
{
  return _cv * temperature;
}

void
SimpleFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = this->e_from_p_T(pressure, temperature);
  de_dp = 0.0;
  de_dT = _cv;
}

void
SimpleFluidProperties::rho_e_dpT(Real pressure,
                                 Real temperature,
                                 Real & rho,
                                 Real & drho_dp,
                                 Real & drho_dT,
                                 Real & e,
                                 Real & de_dp,
                                 Real & de_dT) const
{
  Real density, ddensity_dp, ddensity_dT;
  rho_from_p_T(pressure, temperature, density, ddensity_dp, ddensity_dT);
  rho = density;
  drho_dp = ddensity_dp;
  drho_dT = ddensity_dT;

  Real energy, denergy_dp, denergy_dT;
  e_from_p_T(pressure, temperature, energy, denergy_dp, denergy_dT);
  e = energy;
  de_dp = denergy_dp;
  de_dT = denergy_dT;
}

Real SimpleFluidProperties::mu_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _viscosity;
}

void
SimpleFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;
  dmu_dT = 0.0;
}

void
SimpleFluidProperties::rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  mu = this->mu_from_p_T(pressure, temperature);
}

void
SimpleFluidProperties::rho_mu_dpT(Real pressure,
                                  Real temperature,
                                  Real & rho,
                                  Real & drho_dp,
                                  Real & drho_dT,
                                  Real & mu,
                                  Real & dmu_dp,
                                  Real & dmu_dT) const
{
  this->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  this->mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
}

Real
SimpleFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  return e_from_p_T(pressure, temperature) +
         _pp_coeff * pressure / rho_from_p_T(pressure, temperature);
}

void
SimpleFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h_from_p_T(pressure, temperature);

  Real density, ddensity_dp, ddensity_dT;
  rho_from_p_T(pressure, temperature, density, ddensity_dp, ddensity_dT);

  dh_dp = _pp_coeff / density - _pp_coeff * pressure * ddensity_dp / density / density;
  dh_dT = _cv - _pp_coeff * pressure * ddensity_dT / density / density;
}

Real SimpleFluidProperties::henryConstant(Real /*temperature*/) const { return _henry_constant; }

void
SimpleFluidProperties::henryConstant_dT(Real /*temperature*/, Real & Kh, Real & dKh_dT) const
{
  Kh = _henry_constant;
  dKh_dT = 0.0;
}
