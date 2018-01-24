//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidPropertiesPT.h"

template <>
InputParameters
validParams<IdealGasFluidPropertiesPT>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addParam<Real>("molar_mass", 29.0e-3, "Constant molar mass of the fluid (kg/mol)");
  params.addParam<Real>(
      "thermal_expansion", 3.43e-3, "Constant coefficient of thermal expansion (1/K)");
  params.addParam<Real>(
      "cv", 0.718e3, "Constant specific heat capacity at constant volume (J/kg/K)");
  params.addParam<Real>(
      "cp", 1.005e3, "Constant specific heat capacity at constant pressure (J/kg/K)");
  params.addParam<Real>("thermal_conductivity", 25.68e-3, "Constant thermal conductivity (W/m/K)");
  params.addParam<Real>("specific_entropy", 6.85e3, "Constant specific entropy (J/kg/K)");
  params.addParam<Real>("viscosity", 18.23e-6, "Constant dynamic viscosity (Pa.s)");
  params.addParam<Real>("henry_constant", 0.0, "Henry constant for dissolution in water");
  params.addClassDescription("Fluid properties for an ideal gas");
  return params;
}

IdealGasFluidPropertiesPT::IdealGasFluidPropertiesPT(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters),
    _molar_mass(getParam<Real>("molar_mass")),
    _thermal_expansion(getParam<Real>("thermal_expansion")),
    _cv(getParam<Real>("cv")),
    _cp(getParam<Real>("cp")),
    _thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _specific_entropy(getParam<Real>("specific_entropy")),
    _viscosity(getParam<Real>("viscosity")),
    _henry_constant(getParam<Real>("henry_constant"))
{
}

IdealGasFluidPropertiesPT::~IdealGasFluidPropertiesPT() {}

std::string
IdealGasFluidPropertiesPT::fluidName() const
{
  return "ideal_gas";
}

Real
IdealGasFluidPropertiesPT::molarMass() const
{
  return _molar_mass;
}

Real IdealGasFluidPropertiesPT::beta(Real /*pressure*/, Real /*temperature*/) const
{
  return _thermal_expansion;
}

Real IdealGasFluidPropertiesPT::cp(Real /*pressure*/, Real /*temperature*/) const { return _cp; }

Real IdealGasFluidPropertiesPT::cv(Real /*pressure*/, Real /*temperature*/) const { return _cv; }

Real
IdealGasFluidPropertiesPT::c(Real /*pressure*/, Real temperature) const
{
  return std::sqrt(_cp * _R * temperature / (_cv * _molar_mass));
}

Real IdealGasFluidPropertiesPT::k(Real /*pressure*/, Real /*temperature*/) const
{
  return _thermal_conductivity;
}

void
IdealGasFluidPropertiesPT::k_dpT(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k(pressure, temperature);
  dk_dp = 0;
  dk_dT = 0;
}

Real IdealGasFluidPropertiesPT::k_from_rho_T(Real /*density*/, Real /*temperature*/) const
{
  return _thermal_conductivity;
}

Real IdealGasFluidPropertiesPT::s(Real /*pressure*/, Real /*temperature*/) const
{
  return _specific_entropy;
}

Real
IdealGasFluidPropertiesPT::rho(Real pressure, Real temperature) const
{
  return pressure * _molar_mass / (_R * temperature);
}

void
IdealGasFluidPropertiesPT::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho(pressure, temperature);
  drho_dp = _molar_mass / (_R * temperature);
  drho_dT = -pressure * _molar_mass / (_R * temperature * temperature);
}

Real
IdealGasFluidPropertiesPT::e(Real /*pressure*/, Real temperature) const
{
  return _cv * temperature;
}

void
IdealGasFluidPropertiesPT::e_dpT(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = this->e(pressure, temperature);
  de_dp = 0.0;
  de_dT = _cv;
}

void
IdealGasFluidPropertiesPT::rho_e_dpT(Real pressure,
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

Real IdealGasFluidPropertiesPT::mu(Real /*pressure*/, Real /*temperature*/) const
{
  return _viscosity;
}

void
IdealGasFluidPropertiesPT::mu_dpT(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu(pressure, temperature);
  dmu_dp = 0.0;
  dmu_dT = 0.0;
}

Real IdealGasFluidPropertiesPT::mu_from_rho_T(Real /*density*/, Real /*temperature*/) const
{
  return _viscosity;
}

void
IdealGasFluidPropertiesPT::mu_drhoT_from_rho_T(Real density,
                                               Real temperature,
                                               Real /*ddensity_dT*/,
                                               Real & mu,
                                               Real & dmu_drho,
                                               Real & dmu_dT) const
{
  mu = this->mu_from_rho_T(density, temperature);
  dmu_drho = 0.0;
  dmu_dT = 0.0;
}

Real
IdealGasFluidPropertiesPT::h(Real /*pressure*/, Real temperature) const
{
  return _cp * temperature;
}

void
IdealGasFluidPropertiesPT::h_dpT(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h(pressure, temperature);
  dh_dp = 0.0;
  dh_dT = _cp;
}

Real IdealGasFluidPropertiesPT::henryConstant(Real /*temperature*/) const
{
  return _henry_constant;
}

void
IdealGasFluidPropertiesPT::henryConstant_dT(Real /*temperature*/, Real & Kh, Real & dKh_dT) const
{
  Kh = _henry_constant;
  dKh_dT = 0.0;
}
