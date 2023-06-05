//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleFluidProperties.h"
#include "NewtonInversion.h"

registerMooseObject("FluidPropertiesApp", SimpleFluidProperties);

InputParameters
SimpleFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
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
  params.addParam<Real>("porepressure_coefficient",
                        1.0,
                        "The enthalpy is internal_energy + P / density * "
                        "porepressure_coefficient.  Physically this should be 1.0, "
                        "but analytic solutions are simplified when it is zero");
  params.addClassDescription("Fluid properties for a simple fluid with a constant bulk density");
  return params;
}

SimpleFluidProperties::SimpleFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _molar_mass(getParam<Real>("molar_mass")),
    _thermal_expansion(getParam<Real>("thermal_expansion")),
    _cv(getParam<Real>("cv")),
    _cp(getParam<Real>("cp")),
    _bulk_modulus(getParam<Real>("bulk_modulus")),
    _thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _specific_entropy(getParam<Real>("specific_entropy")),
    _viscosity(getParam<Real>("viscosity")),
    _density0(getParam<Real>("density0")),
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

void
SimpleFluidProperties::beta_from_p_T(
    Real pressure, Real temperature, Real & beta, Real & dbeta_dp, Real & dbeta_dT) const
{
  beta = beta_from_p_T(pressure, temperature);
  dbeta_dp = 0.0;
  dbeta_dT = 0.0;
}

Real SimpleFluidProperties::cp_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cp;
}

void
SimpleFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0.0;
  dcp_dT = 0.0;
}

Real SimpleFluidProperties::cp_from_v_e(Real /*v*/, Real /*e*/) const { return _cp; }

void
SimpleFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_dv = 0.0;
  dcp_de = 0.0;
}

Real SimpleFluidProperties::cv_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cv;
}

void
SimpleFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 0.0;
}

Real SimpleFluidProperties::cv_from_v_e(Real /*v*/, Real /*e*/) const { return _cv; }

void
SimpleFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = 0.0;
  dcv_de = 0.0;
}

Real
SimpleFluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  return std::sqrt(_bulk_modulus / rho_from_p_T(pressure, temperature));
}

void
SimpleFluidProperties::c_from_p_T(
    Real pressure, Real temperature, Real & c, Real & dc_dp, Real & dc_dT) const
{
  c = c_from_p_T(pressure, temperature);
  dc_dp =
      -std::exp(_thermal_expansion * temperature - pressure / _bulk_modulus) / 2 / _density0 /
      std::sqrt(_bulk_modulus *
                std::exp(_thermal_expansion * temperature - pressure / _bulk_modulus) / _density0);
  dc_dT =
      _thermal_expansion * _bulk_modulus *
      std::exp(_thermal_expansion * temperature - pressure / _bulk_modulus) / 2 / _density0 /
      std::sqrt(_bulk_modulus *
                std::exp(_thermal_expansion * temperature - pressure / _bulk_modulus) / _density0);
}

Real
SimpleFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  Real p = p_from_v_e(v, e);
  return std::sqrt(_bulk_modulus / rho_from_p_T(p, T));
}

void
SimpleFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  Real T = T_from_v_e(v, e);
  Real p = p_from_v_e(v, e);

  c = std::sqrt(_bulk_modulus / rho_from_p_T(p, T));

  dc_dv = 0.5 * std::sqrt(_bulk_modulus / v);
  dc_de = 0.0;
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

Real SimpleFluidProperties::k_from_v_e(Real /*v*/, Real /*e*/) const
{
  return _thermal_conductivity;
}

void
SimpleFluidProperties::k_from_v_e(
    Real /*v*/, Real /*e*/, Real & k, Real & dk_dv, Real & dk_de) const
{
  k = _thermal_conductivity;
  dk_dv = 0;
  dk_de = 0;
}

Real SimpleFluidProperties::s_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _specific_entropy;
}

void
SimpleFluidProperties::s_from_p_T(
    Real /*p*/, Real /*T*/, Real & s, Real & ds_dp, Real & ds_dT) const
{
  s = _specific_entropy;
  ds_dp = 0;
  ds_dT = 0;
}

Real SimpleFluidProperties::s_from_h_p(Real /*enthalpy*/, Real /*pressure*/) const
{
  return _specific_entropy;
}

Real SimpleFluidProperties::s_from_v_e(Real /*v*/, Real /*e*/) const { return _specific_entropy; }

void
SimpleFluidProperties::s_from_v_e(
    Real /*v*/, Real /*e*/, Real & s, Real & ds_dv, Real & ds_de) const
{
  s = _specific_entropy;
  ds_dv = 0;
  ds_de = 0;
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

void
SimpleFluidProperties::rho_from_p_T(const DualReal & pressure,
                                    const DualReal & temperature,
                                    DualReal & rho,
                                    DualReal & drho_dp,
                                    DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  drho_dp = rho / _bulk_modulus;
  drho_dT = -_thermal_expansion * rho;
}

Real
SimpleFluidProperties::T_from_v_e(Real /*v*/, Real e) const
{
  // NOTE: while e = _cv * T, h is not equal to _cp * T
  return e / _cv;
}

Real
SimpleFluidProperties::T_from_v_h(Real v, Real h) const
{
  return (std::log(1. / _density0 / v) - h / v / _bulk_modulus) / -_thermal_expansion /
         (1 + _cv / v / _thermal_expansion / _bulk_modulus);
}

void
SimpleFluidProperties::T_from_v_h(Real v, Real h, Real & T, Real & dT_dv, Real & dT_dh) const
{
  T = T_from_v_h(v, h);
  dT_dv =
      (1 / -_thermal_expansion) *
      ((-1. / v + h / v / v / _bulk_modulus) * (1 + _cv / v / _thermal_expansion / _bulk_modulus) -
       (std::log(1. / _density0 / v) - h / v / _bulk_modulus) *
           (-_cv / v / v / _thermal_expansion / _bulk_modulus)) /
      Utility::pow<2>(1 + _cv / v / _thermal_expansion / _bulk_modulus);
  dT_dh = (-1 / v / _bulk_modulus) / -_thermal_expansion /
          (1 + _cv / v / _thermal_expansion / _bulk_modulus);
}

void
SimpleFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

void
SimpleFluidProperties::T_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & T, DualReal & dT_dv, DualReal & dT_de) const
{
  T = SinglePhaseFluidProperties::T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

Real
SimpleFluidProperties::T_from_p_rho(Real p, Real rho) const
{
  mooseAssert(rho > 0, "Density should be positive");
  return (std::log(rho / _density0) - p / _bulk_modulus) / -_thermal_expansion;
}

void
SimpleFluidProperties::T_from_p_rho(Real p, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const
{
  T = T_from_p_rho(p, rho);
  dT_dp = 1 / (_thermal_expansion * _bulk_modulus);
  dT_drho = 1 / (-_thermal_expansion * rho);
}

Real
SimpleFluidProperties::T_from_p_h(Real p, Real h) const
{
  // Likely a better guess than user-selected
  Real T_initial = h / _cp;

  // exponential dependence in rho and linear dependence in e makes it challenging
  auto lambda = [&](Real p, Real current_T, Real & new_rho, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, current_T, new_rho, dh_dp, dh_dT); };
  return FluidPropertiesUtils::NewtonSolve(
             p, h, T_initial, _tolerance, lambda, name() + "::T_from_p_h")
      .first;
}

Real
SimpleFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return _bulk_modulus * (_thermal_expansion * temperature + std::log(1 / (v * _density0)));
}

void
SimpleFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  dp_dv = -_bulk_modulus / v;
  dp_de = _bulk_modulus * _thermal_expansion / _cv;
}

void
SimpleFluidProperties::p_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & p, DualReal & dp_dv, DualReal & dp_de) const
{
  p = SinglePhaseFluidProperties::p_from_v_e(v, e);
  dp_dv = -_bulk_modulus / v;
  dp_de = _bulk_modulus * _thermal_expansion / _cv;
}

Real
SimpleFluidProperties::p_from_v_h(Real v, Real h) const
{
  Real T = T_from_v_h(v, h);
  return _bulk_modulus * (_thermal_expansion * T + std::log(1 / (v * _density0)));
}

void
SimpleFluidProperties::p_from_v_h(Real v, Real h, Real & p, Real & dp_dv, Real & dp_dh) const
{
  Real T, dT_dv, dT_dh;
  T_from_v_h(v, h, T, dT_dv, dT_dh);
  p = _bulk_modulus * (_thermal_expansion * T + std::log(1 / (v * _density0)));
  dp_dv = _bulk_modulus * (_thermal_expansion * dT_dv - 1. / v);
  dp_dh = _bulk_modulus * (_thermal_expansion * dT_dh);
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

Real
SimpleFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  Real T = T_from_p_rho(p, rho);
  return e_from_p_T(p, T);
}

void
SimpleFluidProperties::e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  // get temperature and derivatives
  Real T, dT_dp, dT_drho;
  T_from_p_rho(p, rho, T, dT_dp, dT_drho);

  // get energy and derivatives
  Real de_dT;
  e_from_p_T(p, T, e, de_dp, de_dT);
  de_dp = de_dT * dT_dp + de_dp;
  de_drho = de_dT * dT_drho + de_dp * dT_dp;
}

Real
SimpleFluidProperties::e_from_v_h(Real v, Real h) const
{
  Real T = T_from_v_h(v, h);
  Real p = p_from_v_h(v, h);
  return e_from_p_T(p, T);
}

void
SimpleFluidProperties::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  Real T, dT_dv, dT_dh;
  Real p, dp_dv, dp_dh;
  T_from_v_h(v, h, T, dT_dv, dT_dh);
  p_from_v_h(v, h, p, dp_dv, dp_dh);

  Real de_dp, de_dT;
  e_from_p_T(p, T, e, de_dp, de_dT);
  de_dv = de_dp * dp_dv + de_dT * dT_dv;
  de_dh = de_dp * dp_dh + de_dT * dT_dh;
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

Real SimpleFluidProperties::mu_from_v_e(Real /*v*/, Real /*e*/) const { return _viscosity; }

void
SimpleFluidProperties::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  mu = this->mu_from_v_e(v, e);
  dmu_dv = 0.0;
  dmu_de = 0.0;
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
