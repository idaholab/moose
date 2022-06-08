//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFluidProperties.h"
#include "Conversion.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("FluidPropertiesApp", IdealGasFluidProperties);

InputParameters
IdealGasFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params += NaNInterface::validParams();

  params.addRangeCheckedParam<Real>("gamma", 1.4, "gamma > 1", "gamma value (cp/cv)");
  params.addParam<Real>("molar_mass", 29.0e-3, "Constant molar mass of the fluid (kg/mol)");
  params.addParam<Real>("mu", 18.23e-6, "Dynamic viscosity, Pa.s");
  params.addParam<Real>("k", 25.68e-3, "Thermal conductivity, W/(m-K)");
  params.addParam<Real>("T_c", 0, "Critical temperature, K");
  params.addParam<Real>("rho_c", 0, "Critical density, kg/m3");
  params.addParam<Real>("e_c", 0, "Internal energy at the critical point, J/kg");

  params.addClassDescription("Fluid properties for an ideal gas");

  return params;
}

IdealGasFluidProperties::IdealGasFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    NaNInterface(this),

    _gamma(getParam<Real>("gamma")),
    _molar_mass(getParam<Real>("molar_mass")),

    _R_specific(_R / _molar_mass),
    _cp(_gamma * _R_specific / (_gamma - 1.0)),
    _cv(_cp / _gamma),

    _mu(getParam<Real>("mu")),
    _k(getParam<Real>("k")),

    _T_c(getParam<Real>("T_c")),
    _rho_c(getParam<Real>("rho_c")),
    _e_c(getParam<Real>("e_c"))
{
}

IdealGasFluidProperties::~IdealGasFluidProperties() {}

std::string
IdealGasFluidProperties::fluidName() const
{
  return "ideal_gas";
}

Real
IdealGasFluidProperties::p_from_v_e(Real v, Real e) const
{
  if (v == 0.0)
    return getNaN("Invalid value of specific volume detected (v = " + Moose::stringify(v) + ").");

  return (_gamma - 1.0) * e / v;
}

ADReal
IdealGasFluidProperties::p_from_v_e(const ADReal & v, const ADReal & e) const
{
  if (v.value() == 0.0)
    return getNaN("Invalid value of specific volume detected (v = " + Moose::stringify(v.value()) +
                  ").");

  return (_gamma - 1.0) * e / v;
}

void
IdealGasFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  dp_dv = -(_gamma - 1.0) * e / v / v;
  dp_de = (_gamma - 1.0) / v;
}

void
IdealGasFluidProperties::p_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & p, DualReal & dp_dv, DualReal & dp_de) const
{
  p = p_from_v_e(v, e);
  dp_dv = -(_gamma - 1.0) * e / v / v;
  dp_de = (_gamma - 1.0) / v;
}

Real
IdealGasFluidProperties::T_from_v_e(Real /*v*/, Real e) const
{
  return e / _cv;
}

ADReal
IdealGasFluidProperties::T_from_v_e(const ADReal & /*v*/, const ADReal & e) const
{
  return e / _cv;
}

void
IdealGasFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

void
IdealGasFluidProperties::T_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & T, DualReal & dT_dv, DualReal & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

Real
IdealGasFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);

  const Real c2 = _gamma * _R_specific * T;
  if (c2 < 0)
    return getNaN("Sound speed squared (gamma * R * T) is negative: c2 = " + Moose::stringify(c2) +
                  ".");

  return std::sqrt(c2);
}

ADReal
IdealGasFluidProperties::c_from_v_e(const ADReal & v, const ADReal & e) const
{
  const auto T = T_from_v_e(v, e);

  const auto c2 = _gamma * _R_specific * T;
  if (MetaPhysicL::raw_value(c2) < 0)
    return getNaN("Sound speed squared (gamma * R * T) is negative: c2 = " +
                  Moose::stringify(MetaPhysicL::raw_value(c2)) + ".");

  return std::sqrt(c2);
}

void
IdealGasFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  c = std::sqrt(_gamma * _R_specific * T);

  const Real dc_dT = 0.5 / c * _gamma * _R_specific;
  dc_dv = dc_dT * dT_dv;
  dc_de = dc_dT * dT_de;
}

Real
IdealGasFluidProperties::c_from_p_T(Real /*p*/, Real T) const
{
  return std::sqrt(_cp * _R * T / (_cv * _molar_mass));
}

ADReal
IdealGasFluidProperties::c_from_p_T(const ADReal & /*p*/, const ADReal & T) const
{
  return std::sqrt(_cp * _R * T / (_cv * _molar_mass));
}

void
IdealGasFluidProperties::c_from_p_T(
    const Real /*p*/, const Real T, Real & c, Real & dc_dp, Real & dc_dT) const
{
  c = std::sqrt(_cp * _R * T / (_cv * _molar_mass));
  dc_dp = 0;
  dc_dT = 0.5 / c * _cp * _R / (_cv * _molar_mass);
}

Real IdealGasFluidProperties::cp_from_v_e(Real, Real) const { return _cp; }

void
IdealGasFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_dv = 0.0;
  dcp_de = 0.0;
}

Real IdealGasFluidProperties::cv_from_v_e(Real, Real) const { return _cv; }

void
IdealGasFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = 0.0;
  dcv_de = 0.0;
}

Real IdealGasFluidProperties::gamma_from_v_e(Real, Real) const { return _gamma; }

Real IdealGasFluidProperties::gamma_from_p_T(Real, Real) const { return _gamma; }

Real IdealGasFluidProperties::mu_from_v_e(Real, Real) const { return _mu; }

void
IdealGasFluidProperties::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  mu = this->mu_from_v_e(v, e);
  dmu_dv = 0.0;
  dmu_de = 0.0;
}

Real IdealGasFluidProperties::k_from_v_e(Real, Real) const { return _k; }

void
IdealGasFluidProperties::k_from_v_e(
    Real /*v*/, Real /*e*/, Real & k, Real & dk_dv, Real & dk_de) const
{
  k = _k;
  dk_dv = 0;
  dk_de = 0;
}

Real
IdealGasFluidProperties::s_from_v_e(Real v, Real e) const
{
  const Real T = T_from_v_e(v, e);
  const Real p = p_from_v_e(v, e);
  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
    return getNaN("Negative argument in the ln() function.");
  return _cv * std::log(n);
}

void
IdealGasFluidProperties::s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  Real p, dp_dv, dp_de;
  p_from_v_e(v, e, p, dp_dv, dp_de);

  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
  {
    s = getNaN("Negative argument in the ln() function.");
    ds_dv = getNaN();
    ds_de = getNaN();
  }
  else
  {
    s = _cv * std::log(n);

    const Real dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p, _gamma - 1.0);
    const Real dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p, -_gamma);

    const Real dn_dv = dn_dT * dT_dv + dn_dp * dp_dv;
    const Real dn_de = dn_dT * dT_de + dn_dp * dp_de;

    ds_dv = _cv / n * dn_dv;
    ds_de = _cv / n * dn_de;
  }
}

Real
IdealGasFluidProperties::s_from_p_T(Real p, Real T) const
{
  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
    return getNaN("Negative argument in the ln() function.");
  return _cv * std::log(n);
}

void
IdealGasFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
  {
    s = getNaN("Negative argument in the ln() function.");
    ds_dp = getNaN();
    ds_dT = getNaN();
  }
  else
  {
    s = _cv * std::log(n);

    const Real dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p, _gamma - 1.0);
    const Real dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p, -_gamma);

    ds_dp = _cv / n * dn_dp;
    ds_dT = _cv / n * dn_dT;
  }
}

Real
IdealGasFluidProperties::s_from_h_p(Real h, Real p) const
{
  const Real aux = p * std::pow(h / (_gamma * _cv), -_gamma / (_gamma - 1));
  if (aux <= 0.0)
    return getNaN("Non-positive argument in the ln() function.");
  return -(_gamma - 1) * _cv * std::log(aux);
}

void
IdealGasFluidProperties::s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const
{
  s = s_from_h_p(h, p);

  const Real aux = p * std::pow(h / (_gamma * _cv), -_gamma / (_gamma - 1));
  const Real daux_dh = p * std::pow(h / (_gamma * _cv), -_gamma / (_gamma - 1) - 1) *
                       (-_gamma / (_gamma - 1)) / (_gamma * _cv);
  const Real daux_dp = std::pow(h / (_gamma * _cv), -_gamma / (_gamma - 1));
  ds_dh = -(_gamma - 1) * _cv / aux * daux_dh;
  ds_dp = -(_gamma - 1) * _cv / aux * daux_dp;
}

Real
IdealGasFluidProperties::rho_from_p_s(Real p, Real s) const
{
  const Real aux = (s + _cv * std::log(std::pow(p, _gamma - 1.0))) / _cv;
  const Real T = std::pow(std::exp(aux), 1.0 / _gamma);
  return rho_from_p_T(p, T);
}

void
IdealGasFluidProperties::rho_from_p_s(
    Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const
{
  // T(p,s)
  const Real aux = (s + _cv * std::log(std::pow(p, _gamma - 1.0))) / _cv;
  const Real T = std::pow(std::exp(aux), 1 / _gamma);

  // dT/dp
  const Real dT_dp = 1.0 / _gamma * std::pow(std::exp(aux), 1.0 / _gamma - 1.0) * std::exp(aux) /
                     std::pow(p, _gamma - 1.0) * (_gamma - 1.0) * std::pow(p, _gamma - 2.0);

  // dT/ds
  const Real dT_ds =
      1.0 / _gamma * std::pow(std::exp(aux), 1.0 / _gamma - 1.0) * std::exp(aux) / _cv;

  // Drho/Dp = d/dp[rho(p, T(p,s))] = drho/dp + drho/dT * dT/dp
  Real drho_dp_partial, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp_partial, drho_dT);
  drho_dp = drho_dp_partial + drho_dT * dT_dp;

  // Drho/Ds = d/ds[rho(p, T(p,s))] = drho/dT * dT/ds
  drho_ds = drho_dT * dT_ds;
}

Real
IdealGasFluidProperties::e_from_v_h(Real /*v*/, Real h) const
{
  return h / _gamma;
}

void
IdealGasFluidProperties::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  e = e_from_v_h(v, h);
  de_dv = 0.0;
  de_dh = 1.0 / _gamma;
}

Real
IdealGasFluidProperties::rho_from_p_T(Real p, Real T) const
{
  return p * _molar_mass / (_R * T);
}

ADReal
IdealGasFluidProperties::rho_from_p_T(const ADReal & p, const ADReal & T) const
{
  return p * _molar_mass / (_R * T);
}

void
IdealGasFluidProperties::rho_from_p_T(const DualReal & p,
                                      const DualReal & T,
                                      DualReal & rho,
                                      DualReal & drho_dp,
                                      DualReal & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = _molar_mass / (_R * T);
  drho_dT = -p * _molar_mass / (_R * T * T);
}

void
IdealGasFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = _molar_mass / (_R * T);
  drho_dT = -p * _molar_mass / (_R * T * T);
}

Real
IdealGasFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return p / (_gamma - 1.0) / rho;
}

ADReal
IdealGasFluidProperties::e_from_p_rho(const ADReal & p, const ADReal & rho) const
{
  return p / (_gamma - 1.0) / rho;
}

void
IdealGasFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e = e_from_p_rho(p, rho);
  de_dp = 1.0 / (_gamma - 1.0) / rho;
  de_drho = -p / (_gamma - 1.0) / rho / rho;
}

void
IdealGasFluidProperties::e_from_p_rho(
    const ADReal & p, const ADReal & rho, ADReal & e, ADReal & de_dp, ADReal & de_drho) const
{
  e = e_from_p_rho(p, rho);
  de_dp = 1.0 / (_gamma - 1.0) / rho;
  de_drho = -p / (_gamma - 1.0) / rho / rho;
}

Real
IdealGasFluidProperties::e_from_T_v(Real T, Real /*v*/) const
{
  return _cv * T;
}

void
IdealGasFluidProperties::e_from_T_v(Real T, Real /*v*/, Real & e, Real & de_dT, Real & de_dv) const
{
  e = _cv * T;
  de_dT = _cv;
  de_dv = 0.0;
}

ADReal
IdealGasFluidProperties::e_from_T_v(const ADReal & T, const ADReal & /*v*/) const
{
  return _cv * T;
}

void
IdealGasFluidProperties::e_from_T_v(
    const ADReal & T, const ADReal & /*v*/, ADReal & e, ADReal & de_dT, ADReal & de_dv) const
{
  e = _cv * T;
  de_dT = _cv;
  de_dv = 0.0;
}

Real
IdealGasFluidProperties::p_from_T_v(Real T, Real v) const
{
  return (_gamma - 1.0) * _cv * T / v;
}

void
IdealGasFluidProperties::p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const
{
  p = (_gamma - 1.0) * _cv * T / v;
  dp_dT = (_gamma - 1.0) * _cv / v;
  dp_dv = -(_gamma - 1.0) * _cv * T / (v * v);
}

Real
IdealGasFluidProperties::h_from_T_v(Real T, Real /*v*/) const
{
  return _gamma * _cv * T;
}

void
IdealGasFluidProperties::h_from_T_v(Real T, Real /*v*/, Real & h, Real & dh_dT, Real & dh_dv) const
{
  h = _gamma * _cv * T;
  dh_dT = _gamma * _cv;
  dh_dv = 0.0;
}

Real
IdealGasFluidProperties::s_from_T_v(Real T, Real v) const
{
  Real p = p_from_T_v(T, v);
  return s_from_p_T(p, T);
}

void
IdealGasFluidProperties::s_from_T_v(Real T, Real v, Real & s, Real & ds_dT, Real & ds_dv) const
{
  Real p, dp_dT_v, dp_dv_T;
  Real ds_dp_T, ds_dT_p;
  p_from_T_v(T, v, p, dp_dT_v, dp_dv_T);
  s_from_p_T(p, T, s, ds_dp_T, ds_dT_p);
  ds_dT = ds_dT_p + ds_dp_T * dp_dT_v;
  ds_dv = ds_dp_T * dp_dv_T;
}

Real IdealGasFluidProperties::cv_from_T_v(Real /*T*/, Real /*v*/) const { return _cv; }

Real IdealGasFluidProperties::e_spndl_from_v(Real /*v*/) const { return _e_c; }

void
IdealGasFluidProperties::v_e_spndl_from_T(Real /*T*/, Real & v, Real & e) const
{
  v = 1. / _rho_c;
  e = _e_c;
}

Real
IdealGasFluidProperties::h_from_p_T(Real /*p*/, Real T) const
{
  return _cp * T;
}

void
IdealGasFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  dh_dp = 0.0;
  dh_dT = _cp;
}

Real
IdealGasFluidProperties::e_from_p_T(Real /*p*/, Real T) const
{
  return _cv * T;
}

void
IdealGasFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(p, T);
  de_dp = 0.0;
  de_dT = _cv;
}

Real
IdealGasFluidProperties::p_from_h_s(Real h, Real s) const
{
  return std::pow(h / (_gamma * _cv), _gamma / (_gamma - 1.0)) *
         std::exp(-s / ((_gamma - 1.0) * _cv));
}

void
IdealGasFluidProperties::p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const
{
  p = p_from_h_s(h, s);
  dp_dh = _gamma / (_gamma - 1.0) / (_gamma * _cv) *
          std::pow(h / (_gamma * _cv), 1.0 / (_gamma - 1.0)) *
          std::exp(-s / ((_gamma - 1.0) * _cv));
  dp_ds = std::pow(h / (_gamma * _cv), _gamma / (_gamma - 1)) *
          std::exp(-s / ((_gamma - 1) * _cv)) / ((1 - _gamma) * _cv);
}

Real
IdealGasFluidProperties::g_from_v_e(Real v, Real e) const
{
  // g(p,T) for SGEOS is given by Equation (37) in the following reference:
  //
  // Ray A. Berry, Richard Saurel, Olivier LeMetayer
  // The discrete equation method (DEM) for fully compressible, two-phase flows in
  //   ducts of spatially varying cross-section
  // Nuclear Engineering and Design 240 (2010) p. 3797-3818
  //
  const Real p = p_from_v_e(v, e);
  const Real T = T_from_v_e(v, e);

  return _gamma * _cv * T - _cv * T * std::log(std::pow(T, _gamma) / std::pow(p, _gamma - 1.0));
}

Real
IdealGasFluidProperties::molarMass() const
{
  return _molar_mass;
}

Real
IdealGasFluidProperties::criticalTemperature() const
{
  return _T_c;
}

Real
IdealGasFluidProperties::criticalDensity() const
{
  return _rho_c;
}

Real
IdealGasFluidProperties::criticalInternalEnergy() const
{
  return _e_c;
}

Real
IdealGasFluidProperties::T_from_p_h(Real, Real h) const
{
  return h / _gamma / _cv;
}

void
IdealGasFluidProperties::T_from_p_h(Real /*p*/, Real h, Real & T, Real & dT_dp, Real & dT_dh) const
{
  T = h / (_gamma * _cv);
  dT_dp = 0;
  dT_dh = 1.0 / (_gamma * _cv);
}

Real IdealGasFluidProperties::cv_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _cv;
}

void
IdealGasFluidProperties::cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(p, T);
  dcv_dp = 0.0;
  dcv_dT = 0.0;
}

Real IdealGasFluidProperties::cp_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _cp;
}

void
IdealGasFluidProperties::cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(p, T);
  dcp_dp = 0.0;
  dcp_dT = 0.0;
}

Real IdealGasFluidProperties::mu_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _mu;
}

void
IdealGasFluidProperties::mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(p, T);
  dmu_dp = 0.0;
  dmu_dT = 0.0;
}

Real IdealGasFluidProperties::k_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _k;
}

void
IdealGasFluidProperties::k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(p, T);
  dk_dp = 0.0;
  dk_dT = 0.0;
}

Real IdealGasFluidProperties::pp_sat_from_p_T(Real /*p*/, Real /*T*/) const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented. Use a real fluid property class!");
}
