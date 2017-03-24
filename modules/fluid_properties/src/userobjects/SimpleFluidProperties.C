/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SimpleFluidProperties.h"

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
  params.addClassDescription("Fluid properties for a simple fluid.  density=density0 * exp(P / "
                             "bulk_modulus - thermal_expansion * T), internal_energy = cv * T, "
                             "enthalpy = cp * T");
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
    _henry_constant(getParam<Real>("henry_constant"))
{
}

SimpleFluidProperties::~SimpleFluidProperties() {}

Real
SimpleFluidProperties::molarMass() const
{
  return _molar_mass;
}

Real SimpleFluidProperties::beta(Real /*pressure*/, Real /*temperature*/) const
{
  return _thermal_expansion;
}

Real SimpleFluidProperties::cp(Real /*pressure*/, Real /*temperature*/) const { return _cp; }

Real SimpleFluidProperties::cv(Real /*pressure*/, Real /*temperature*/) const { return _cv; }

Real
SimpleFluidProperties::c(Real pressure, Real temperature) const
{
  return std::sqrt(_bulk_modulus / rho(pressure, temperature));
}

Real SimpleFluidProperties::k(Real /*pressure*/, Real /*temperature*/) const
{
  return _thermal_conductivity;
}

Real SimpleFluidProperties::s(Real /*pressure*/, Real /*temperature*/) const
{
  return _specific_entropy;
}

Real
SimpleFluidProperties::rho(Real pressure, Real temperature) const
{
  return _density0 * std::exp(pressure / _bulk_modulus - _thermal_expansion * temperature);
}

void
SimpleFluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho(pressure, temperature);
  drho_dp = rho / _bulk_modulus;
  drho_dT = -_thermal_expansion * rho;
}

Real
SimpleFluidProperties::e(Real /*pressure*/, Real temperature) const
{
  return _cv * temperature;
}

void
SimpleFluidProperties::e_dpT(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = this->e(pressure, temperature);
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
  rho_dpT(pressure, temperature, density, ddensity_dp, ddensity_dT);
  rho = density;
  drho_dp = ddensity_dp;
  drho_dT = ddensity_dT;

  Real energy, denergy_dp, denergy_dT;
  e_dpT(pressure, temperature, energy, denergy_dp, denergy_dT);
  e = energy;
  de_dp = denergy_dp;
  de_dT = denergy_dT;
}

Real SimpleFluidProperties::mu(Real /*density*/, Real /*temperature*/) const { return _viscosity; }

void
SimpleFluidProperties::mu_drhoT(
    Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const
{
  mu = this->mu(density, temperature);
  dmu_drho = 0.0;
  dmu_dT = 0.0;
}

Real
SimpleFluidProperties::h(Real /*pressure*/, Real temperature) const
{
  return _cp * temperature;
}

void
SimpleFluidProperties::h_dpT(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h(pressure, temperature);
  dh_dp = 0.0;
  dh_dT = _cp;
}

Real SimpleFluidProperties::henryConstant(Real /*temperature*/) const { return _henry_constant; }

void
SimpleFluidProperties::henryConstant_dT(Real /*temperature*/, Real & Kh, Real & dKh_dT) const
{
  Kh = _henry_constant;
  dKh_dT = 0.0;
}
