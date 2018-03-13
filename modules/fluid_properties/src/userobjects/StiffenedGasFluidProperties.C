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

template <>
InputParameters
validParams<StiffenedGasFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();
  params.addRequiredParam<Real>("gamma", "Heat capacity ratio");
  params.addRequiredParam<Real>("cv", "Constant volume specific heat");
  params.addRequiredParam<Real>("q", "Parameter defining zero point of internal energy");
  params.addRequiredParam<Real>("p_inf", "Stiffness parameter");
  params.addParam<Real>("q_prime", 0, "Parameter");
  params.addParam<Real>("mu", 1.e-3, "Dynamic viscosity, Pa.s");
  params.addParam<Real>("k", 0.6, "Thermal conductivity, W/(m-K)");
  params.addClassDescription("Fluid properties for a stiffened gas");
  return params;
}

StiffenedGasFluidProperties::StiffenedGasFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _gamma(getParam<Real>("gamma")),
    _cv(getParam<Real>("cv")),
    _q(getParam<Real>("q")),
    _q_prime(getParam<Real>("q_prime")),
    _p_inf(getParam<Real>("p_inf")),
    _mu(getParam<Real>("mu")),
    _k(getParam<Real>("k"))
{
  if (_cv == 0.0)
    mooseError(name(), ": cv cannot be zero.");
  _cp = _cv * _gamma;
}

StiffenedGasFluidProperties::~StiffenedGasFluidProperties() {}

Real
StiffenedGasFluidProperties::p_from_v_e(Real v, Real e) const
{
  return (_gamma - 1.0) * (e - _q) / v - _gamma * _p_inf;
}

void
StiffenedGasFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  dp_dv = -(_gamma - 1.0) * (e - _q) / v / v;
  dp_de = (_gamma - 1.0) / v;
}

Real
StiffenedGasFluidProperties::T_from_v_e(Real v, Real e) const
{
  return (1.0 / _cv) * (e - _q - _p_inf * v);
}

void
StiffenedGasFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = -_p_inf / _cv;
  dT_de = 1.0 / _cv;
}

Real
StiffenedGasFluidProperties::c_from_v_e(Real v, Real e) const
{
  return std::sqrt(_gamma * (p_from_v_e(v, e) + _p_inf) * v);
}

void
StiffenedGasFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  Real p, dp_dv, dp_de;
  p_from_v_e(v, e, p, dp_dv, dp_de);

  c = c_from_v_e(v, e);
  const Real dc_dp = 0.5 / c * _gamma * v;
  const Real dc_dv_partial = 0.5 / c * _gamma * (p + _p_inf);

  dc_dv = dc_dv_partial + dc_dp * dp_dv;
  dc_de = dc_dp * dp_de;
}

Real StiffenedGasFluidProperties::cp_from_v_e(Real, Real) const { return _cp; }

Real StiffenedGasFluidProperties::cv_from_v_e(Real, Real) const { return _cv; }

Real StiffenedGasFluidProperties::mu_from_v_e(Real, Real) const { return _mu; }

Real StiffenedGasFluidProperties::k_from_v_e(Real, Real) const { return _k; }

Real
StiffenedGasFluidProperties::s_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  Real p = p_from_v_e(v, e);
  Real n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0);
  if (n <= 0.0)
    throw MooseException(name() + ": Negative argument in the ln() function.");
  return _cv * std::log(n) + _q_prime;
}

void
StiffenedGasFluidProperties::s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  Real p, dp_dv, dp_de;
  p_from_v_e(v, e, p, dp_dv, dp_de);

  const Real n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0);
  if (n <= 0.0)
    throw MooseException(name() + ": Negative argument in the ln() function.");

  s = _cv * std::log(n) + _q_prime;

  const Real dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p + _p_inf, _gamma - 1.0);
  const Real dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p + _p_inf, -_gamma);

  const Real dn_dv = dn_dT * dT_dv + dn_dp * dp_dv;
  const Real dn_de = dn_dT * dT_de + dn_dp * dp_de;

  ds_dv = _cv / n * dn_dv;
  ds_de = _cv / n * dn_de;
}

Real
StiffenedGasFluidProperties::s_from_h_p(Real h, Real p) const
{
  const Real aux = (p + _p_inf) * std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1));
  if (aux <= 0.0)
    throw MooseException(name() + ": Non-positive argument in the ln() function.");
  return _q_prime - (_gamma - 1) * _cv * std::log(aux);
}

void
StiffenedGasFluidProperties::s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const
{
  const Real aux = (p + _p_inf) * std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1));
  if (aux <= 0.0)
    throw MooseException(name() + ": Non-positive argument in the ln() function.");

  const Real daux_dh = (p + _p_inf) *
                       std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1) - 1) *
                       (-_gamma / (_gamma - 1)) / (_gamma * _cv);
  const Real daux_dp = std::pow((h - _q) / (_gamma * _cv), -_gamma / (_gamma - 1));

  s = _q_prime - (_gamma - 1) * _cv * std::log(aux);
  ds_dh = -(_gamma - 1) * _cv / aux * daux_dh;
  ds_dp = -(_gamma - 1) * _cv / aux * daux_dp;
}

Real
StiffenedGasFluidProperties::rho_from_p_s(Real p, Real s) const
{
  Real a = (s - _q_prime + _cv * std::log(std::pow(p + _p_inf, _gamma - 1.0))) / _cv;
  Real T = std::pow(std::exp(a), 1.0 / _gamma);
  return rho_from_p_T(p, T);
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
  e = e_from_v_h(v, h);
  de_dv = _p_inf;
  de_dh = 1.0 / _gamma;
}

Real
StiffenedGasFluidProperties::beta_from_p_T(Real p, Real T) const
{
  // The volumetric thermal expansion coefficient is defined as
  //   1/v dv/dT)_p
  // It is the fractional change rate of volume with respect to temperature change
  // at constant pressure. Here it is coded as
  //   - 1/rho drho/dT)_p
  // using chain rule with v = v(rho)

  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  return -drho_dT / rho;
}

Real
StiffenedGasFluidProperties::rho_from_p_T(Real p, Real T) const
{
  mooseAssert(((_gamma - 1.0) * _cv * T) != 0.0, "Invalid gamma or cv or temperature detected!");
  return (p + _p_inf) / ((_gamma - 1.0) * _cv * T);
}

void
StiffenedGasFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = 1. / ((_gamma - 1.0) * _cv * T);
  drho_dT = -(p + _p_inf) / ((_gamma - 1.0) * _cv * T * T);
}

Real
StiffenedGasFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  mooseAssert((_gamma - 1.0) * rho != 0., "Invalid gamma or density detected!");
  return (p + _gamma * _p_inf) / ((_gamma - 1.0) * rho) + _q;
}

void
StiffenedGasFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e = e_from_p_rho(p, rho);
  de_dp = 1.0 / ((_gamma - 1.0) * rho);
  de_drho = -(p + _gamma * _p_inf) / ((_gamma - 1.0) * rho * rho);
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

Real StiffenedGasFluidProperties::gamma_from_v_e(Real /*v*/, Real /*e*/) const { return _gamma; }

Real
StiffenedGasFluidProperties::c2_from_p_rho(Real pressure, Real rho) const
{
  return _gamma * (pressure + _p_inf) / rho;
}
