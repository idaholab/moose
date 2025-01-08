//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StiffenedGasFluidProperties.h"

registerMooseObject("FluidPropertiesApp", StiffenedGasFluidProperties);

InputParameters
StiffenedGasFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params += NaNInterface::validParams();
  params.addParam<bool>(
      "allow_nonphysical_states", true, "Allows for non-physical states, e.g., negative density.");
  params.addRequiredParam<Real>("gamma", "Heat capacity ratio");
  params.addRequiredParam<Real>("cv", "Constant volume specific heat");
  params.addRequiredParam<Real>("q", "Parameter defining zero point of internal energy");
  params.addRequiredParam<Real>("p_inf", "Stiffness parameter");
  params.addParam<Real>("q_prime", 0, "Parameter");
  params.addParam<Real>("mu", 1.e-3, "Dynamic viscosity, Pa.s");
  params.addParam<Real>("k", 0.6, "Thermal conductivity, W/(m-K)");
  params.addParam<Real>("M", 0, "Molar mass, kg/mol");
  params.addParam<Real>("T_c", 0, "Critical temperature, K");
  params.addParam<Real>("rho_c", 0, "Critical density, kg/m3");
  params.addParam<Real>("e_c", 0, "Internal energy at the critical point, J/kg");
  params.addClassDescription("Fluid properties for a stiffened gas");
  return params;
}

StiffenedGasFluidProperties::StiffenedGasFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    NaNInterface(this),
    _allow_nonphysical_states(getParam<bool>("allow_nonphysical_states")),
    _gamma(getParam<Real>("gamma")),
    _cv(getParam<Real>("cv")),
    _q(getParam<Real>("q")),
    _q_prime(getParam<Real>("q_prime")),
    _p_inf(getParam<Real>("p_inf")),
    _mu(getParam<Real>("mu")),
    _k(getParam<Real>("k")),
    _molar_mass(getParam<Real>("M")),
    _T_c(getParam<Real>("T_c")),
    _rho_c(getParam<Real>("rho_c")),
    _e_c(getParam<Real>("e_c"))
{
  if (_cv == 0.0)
    mooseError("cv cannot be zero.");
  _cp = _cv * _gamma;
}

StiffenedGasFluidProperties::~StiffenedGasFluidProperties() {}

Real
StiffenedGasFluidProperties::p_from_v_e(Real v, Real e) const
{
  return p_from_v_e_template(v, e);
}

void
StiffenedGasFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p_from_v_e_template(v, e, p, dp_dv, dp_de);
}

ADReal
StiffenedGasFluidProperties::p_from_v_e(const ADReal & v, const ADReal & e) const
{
  return p_from_v_e_template(v, e);
}

void
StiffenedGasFluidProperties::p_from_v_e(
    const ADReal & v, const ADReal & e, ADReal & p, ADReal & dp_dv, ADReal & dp_de) const
{
  p_from_v_e_template(v, e, p, dp_dv, dp_de);
}

Real
StiffenedGasFluidProperties::T_from_v_e(Real v, Real e) const
{
  return T_from_v_e_template(v, e);
}

void
StiffenedGasFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T_from_v_e_template(v, e, T, dT_dv, dT_de);
}

ADReal
StiffenedGasFluidProperties::T_from_v_e(const ADReal & v, const ADReal & e) const
{
  return T_from_v_e_template(v, e);
}

void
StiffenedGasFluidProperties::T_from_v_e(
    const ADReal & v, const ADReal & e, ADReal & p, ADReal & dp_dv, ADReal & dp_de) const
{
  T_from_v_e_template(v, e, p, dp_dv, dp_de);
}

Real
StiffenedGasFluidProperties::T_from_p_h(Real v, Real e) const
{
  return T_from_p_h_template(v, e);
}

void
StiffenedGasFluidProperties::T_from_p_h(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T_from_p_h_template(v, e, T, dT_dv, dT_de);
}

ADReal
StiffenedGasFluidProperties::T_from_p_h(const ADReal & v, const ADReal & e) const
{
  return T_from_p_h_template(v, e);
}

void
StiffenedGasFluidProperties::T_from_p_h(
    const ADReal & v, const ADReal & e, ADReal & p, ADReal & dp_dv, ADReal & dp_de) const
{
  T_from_p_h_template(v, e, p, dp_dv, dp_de);
}

Real
StiffenedGasFluidProperties::c_from_v_e(Real v, Real e) const
{
  if (_allow_nonphysical_states)
    return std::sqrt(_gamma * (p_from_v_e(v, e) + _p_inf) * v);
  else
  {
    const Real radicant = _gamma * (p_from_v_e(v, e) + _p_inf) * v;
    if (radicant < 0.)
    {
      return getNaN();
    }
    else
    {
      return std::sqrt(radicant);
    }
  }
}

void
StiffenedGasFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  if (_allow_nonphysical_states)
  {
    Real p, dp_dv, dp_de;
    p_from_v_e(v, e, p, dp_dv, dp_de);

    c = std::sqrt(_gamma * (p_from_v_e(v, e) + _p_inf) * v);
    const Real dc_dp = 0.5 / c * _gamma * v;
    const Real dc_dv_partial = 0.5 / c * _gamma * (p + _p_inf);

    dc_dv = dc_dv_partial + dc_dp * dp_dv;
    dc_de = dc_dp * dp_de;
  }
  else
  {
    const Real radicant = _gamma * (p_from_v_e(v, e) + _p_inf) * v;
    if (radicant < 0.)
    {
      c = getNaN();
      dc_dv = getNaN();
      dc_de = getNaN();
    }
    else
    {
      Real p, dp_dv, dp_de;
      p_from_v_e(v, e, p, dp_dv, dp_de);
      c = std::sqrt(radicant);
      const Real dc_dp = 0.5 / c * _gamma * v;
      const Real dc_dv_partial = 0.5 / c * _gamma * (p + _p_inf);

      dc_dv = dc_dv_partial + dc_dp * dp_dv;
      dc_de = dc_dp * dp_de;
    }
  }
}

Real StiffenedGasFluidProperties::cp_from_v_e(Real, Real) const { return _cp; }

void
StiffenedGasFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_dv = 0.0;
  dcp_de = 0.0;
}

Real StiffenedGasFluidProperties::cv_from_v_e(Real, Real) const { return _cv; }

void
StiffenedGasFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = 0.0;
  dcv_de = 0.0;
}

Real StiffenedGasFluidProperties::mu_from_v_e(Real, Real) const { return _mu; }

void
StiffenedGasFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  mu = this->mu_from_v_e(v, e);
  dmu_dv = 0.0;
  dmu_de = 0.0;
}

Real StiffenedGasFluidProperties::k_from_v_e(Real, Real) const { return _k; }

void
StiffenedGasFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  k = k_from_v_e(v, e);
  dk_dv = 0.0;
  dk_de = 0.0;
}

Real
StiffenedGasFluidProperties::s_from_v_e(Real v, Real e) const
{
  return s_from_v_e_template(v, e);
}

void
StiffenedGasFluidProperties::s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  s_from_v_e_template(v, e, s, ds_dv, ds_de);
}

ADReal
StiffenedGasFluidProperties::s_from_v_e(const ADReal & v, const ADReal & e) const
{
  return s_from_v_e_template(v, e);
}

void
StiffenedGasFluidProperties::s_from_v_e(
    const ADReal & v, const ADReal & e, ADReal & s, ADReal & ds_dv, ADReal & ds_de) const
{
  s_from_v_e_template(v, e, s, ds_dv, ds_de);
}

Real
StiffenedGasFluidProperties::s_from_h_p(Real h, Real p) const
{
  const Real aux = (p + _p_inf) * std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1));
  if (aux <= 0.0)
    return getNaN();
  else
    return _q_prime - (_gamma - 1) * _cv * std::log(aux);
}

void
StiffenedGasFluidProperties::s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const
{
  const Real aux = (p + _p_inf) * std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1));
  if (aux <= 0.0)
  {
    s = getNaN();
    ds_dh = getNaN();
    ds_dp = getNaN();
  }
  else
  {
    const Real daux_dh = (p + _p_inf) *
                         std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1) - 1) *
                         (-_gamma / (_gamma - 1)) / (_gamma * _cv);
    const Real daux_dp = std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1));

    s = _q_prime - (_gamma - 1) * _cv * std::log(aux);
    ds_dh = -(_gamma - 1) * _cv / aux * daux_dh;
    ds_dp = -(_gamma - 1) * _cv / aux * daux_dp;
  }
}

Real
StiffenedGasFluidProperties::s_from_p_T(Real p, Real T) const
{
  return s_from_p_T_template(p, T);
}

void
StiffenedGasFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  s_from_p_T_template(p, T, s, ds_dp, ds_dT);
}

ADReal
StiffenedGasFluidProperties::s_from_p_T(const ADReal & p, const ADReal & T) const
{
  return s_from_p_T_template(p, T);
}

void
StiffenedGasFluidProperties::s_from_p_T(
    const ADReal & p, const ADReal & T, ADReal & s, ADReal & ds_dp, ADReal & ds_dT) const
{
  s_from_p_T_template(p, T, s, ds_dp, ds_dT);
}

Real
StiffenedGasFluidProperties::rho_from_p_s(Real p, Real s) const
{
  Real a = (s - _q_prime + _cv * std::log(std::pow(p + _p_inf, _gamma - 1.0))) / _cv;
  Real T = std::pow(std::exp(a), 1.0 / _gamma);
  Real rho = rho_from_p_T(p, T);
  if (!_allow_nonphysical_states && rho <= 0.)
    return getNaN();
  else
    return rho;
}

void
StiffenedGasFluidProperties::rho_from_p_s(
    Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const
{
  // T(p,s)
  const Real aux = (s - _q_prime + _cv * std::log(std::pow(p + _p_inf, _gamma - 1.0))) / _cv;
  const Real T = std::pow(std::exp(aux), 1 / _gamma);

  // dT/dp
  const Real dT_dp = 1.0 / _gamma * std::pow(std::exp(aux), 1.0 / _gamma - 1.0) * std::exp(aux) /
                     std::pow(p + _p_inf, _gamma - 1.0) * (_gamma - 1.0) *
                     std::pow(p + _p_inf, _gamma - 2.0);

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
StiffenedGasFluidProperties::e_from_v_h(Real v, Real h) const
{
  return (h + (_gamma - 1.0) * _q + _gamma * _p_inf * v) / _gamma;
}

void
StiffenedGasFluidProperties::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  e = (h + (_gamma - 1.0) * _q + _gamma * _p_inf * v) / _gamma;
  de_dv = _p_inf;
  de_dh = 1.0 / _gamma;
}

Real
StiffenedGasFluidProperties::rho_from_p_T(Real p, Real T) const
{
  return rho_from_p_T_template(p, T);
}

void
StiffenedGasFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho_from_p_T_template(p, T, rho, drho_dp, drho_dT);
}

ADReal
StiffenedGasFluidProperties::rho_from_p_T(const ADReal & p, const ADReal & T) const
{
  return rho_from_p_T_template(p, T);
}

void
StiffenedGasFluidProperties::rho_from_p_T(
    const ADReal & p, const ADReal & T, ADReal & rho, ADReal & drho_dp, ADReal & drho_dT) const
{
  rho_from_p_T_template(p, T, rho, drho_dp, drho_dT);
}

Real
StiffenedGasFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_rho_template(p, rho);
}

void
StiffenedGasFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e_from_p_rho_template(p, rho, e, de_dp, de_drho);
}

ADReal
StiffenedGasFluidProperties::e_from_p_rho(const ADReal & p, const ADReal & rho) const
{
  return e_from_p_rho_template(p, rho);
}

void
StiffenedGasFluidProperties::e_from_p_rho(
    const ADReal & p, const ADReal & rho, ADReal & e, ADReal & de_dp, ADReal & de_drho) const
{
  e_from_p_rho_template(p, rho, e, de_dp, de_drho);
}

Real
StiffenedGasFluidProperties::e_from_T_v(Real T, Real v) const
{
  return _cv * T + _q + _p_inf * v;
}

void
StiffenedGasFluidProperties::e_from_T_v(Real T, Real v, Real & e, Real & de_dT, Real & de_dv) const
{
  e = _cv * T + _q + _p_inf * v;
  de_dT = _cv;
  de_dv = _p_inf;
}

Real
StiffenedGasFluidProperties::p_from_T_v(Real T, Real v) const
{
  Real e = e_from_T_v(T, v);
  return p_from_v_e(v, e);
}

void
StiffenedGasFluidProperties::p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const
{
  Real e, de_dT_v, de_dv_T, dp_dv_e, dp_de_v;
  e_from_T_v(T, v, e, de_dT_v, de_dv_T);
  p_from_v_e(v, e, p, dp_dv_e, dp_de_v);
  dp_dT = dp_de_v * de_dT_v;
  dp_dv = dp_dv_e + dp_de_v * de_dv_T;
}

Real
StiffenedGasFluidProperties::h_from_T_v(Real T, Real /*v*/) const
{
  return _gamma * _cv * T + _q;
}

void
StiffenedGasFluidProperties::h_from_T_v(
    Real T, Real /*v*/, Real & h, Real & dh_dT, Real & dh_dv) const
{
  h = _gamma * _cv * T + _q;
  dh_dT = _gamma * _cv;
  dh_dv = 0.0;
}

Real
StiffenedGasFluidProperties::s_from_T_v(Real T, Real v) const
{
  Real e = e_from_T_v(T, v);
  return s_from_v_e(v, e);
}

void
StiffenedGasFluidProperties::s_from_T_v(Real T, Real v, Real & s, Real & ds_dT, Real & ds_dv) const
{
  Real e, de_dT_v, de_dv_T, ds_dv_e, ds_de_v;
  e_from_T_v(T, v, e, de_dT_v, de_dv_T);
  s_from_v_e(v, e, s, ds_dv_e, ds_de_v);
  ds_dT = ds_de_v * de_dT_v;
  ds_dv = ds_dv_e + ds_de_v * de_dv_T;
}

Real StiffenedGasFluidProperties::cv_from_T_v(Real /*T*/, Real /*v*/) const { return _cv; }

Real StiffenedGasFluidProperties::e_spndl_from_v(Real /*v*/) const { return _e_c; }

void
StiffenedGasFluidProperties::v_e_spndl_from_T(Real /*T*/, Real & v, Real & e) const
{
  v = 1. / _rho_c;
  e = _e_c;
}

Real
StiffenedGasFluidProperties::h_from_p_T(Real /*p*/, Real T) const
{
  return _gamma * _cv * T + _q;
}

void
StiffenedGasFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  dh_dp = 0.0;
  dh_dT = _gamma * _cv;
}

Real
StiffenedGasFluidProperties::e_from_p_T(Real p, Real T) const
{
  return (p + _gamma * _p_inf) / (p + _p_inf) * _cv * T + _q;
}

void
StiffenedGasFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(p, T);
  de_dp = (1. - _gamma) * _p_inf / (p + _p_inf) / (p + _p_inf) * _cv * T;
  de_dT = (p + _gamma * _p_inf) / (p + _p_inf) * _cv;
}

Real
StiffenedGasFluidProperties::p_from_h_s(Real h, Real s) const
{
  return std::pow((h - _q) / (_gamma * _cv), _gamma / (_gamma - 1.0)) *
             std::exp((_q_prime - s) / ((_gamma - 1.0) * _cv)) -
         _p_inf;
}

void
StiffenedGasFluidProperties::p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const
{
  p = p_from_h_s(h, s);
  dp_dh = _gamma / (_gamma - 1.0) / (_gamma * _cv) *
          std::pow((h - _q) / (_gamma * _cv), 1.0 / (_gamma - 1.0)) *
          std::exp((_q_prime - s) / ((_gamma - 1.0) * _cv));
  dp_ds = std::pow((h - _q) / (_gamma * _cv), _gamma / (_gamma - 1)) *
          std::exp((_q_prime - s) / ((_gamma - 1) * _cv)) / ((1 - _gamma) * _cv);
}

Real
StiffenedGasFluidProperties::g_from_v_e(Real v, Real e) const
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

  return (_gamma * _cv - _q_prime) * T -
         _cv * T * std::log(std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0)) + _q;
}

Real
StiffenedGasFluidProperties::c2_from_p_rho(Real pressure, Real rho) const
{
  return _gamma * (pressure + _p_inf) / rho;
}

Real
StiffenedGasFluidProperties::molarMass() const
{
  return _molar_mass;
}

Real
StiffenedGasFluidProperties::criticalTemperature() const
{
  return _T_c;
}

Real
StiffenedGasFluidProperties::criticalDensity() const
{
  return _rho_c;
}

Real
StiffenedGasFluidProperties::criticalInternalEnergy() const
{
  return _e_c;
}

Real StiffenedGasFluidProperties::cv_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _cv;
}

void
StiffenedGasFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 0.0;
}

Real StiffenedGasFluidProperties::cp_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _cp;
}

void
StiffenedGasFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0.0;
  dcp_dT = 0.0;
}

Real StiffenedGasFluidProperties::mu_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _mu;
}

void
StiffenedGasFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;
  dmu_dT = 0.0;
}

Real StiffenedGasFluidProperties::k_from_p_T(Real /* pressure */, Real /* temperature */) const
{
  return _k;
}

void
StiffenedGasFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k_from_p_T(pressure, temperature);
  dk_dp = 0.0;
  dk_dT = 0.0;
}
Real
StiffenedGasFluidProperties::beta_from_p_T(Real /* pressure */, Real temperature) const
{
  return 1 / temperature;
}
void
StiffenedGasFluidProperties::beta_from_p_T(
    Real pressure, Real temperature, Real & beta, Real & dbeta_dp, Real & dbeta_dT) const
{
  beta = this->beta_from_p_T(pressure, temperature);
  dbeta_dp = 0.0;
  dbeta_dT = -1 / (temperature * temperature);
}

Real StiffenedGasFluidProperties::pp_sat_from_p_T(Real /*p*/, Real /*T*/) const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented. Use a real fluid property class!");
}
