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

registerMooseObject("FluidPropertiesApp", IdealGasFluidProperties);

template <>
InputParameters
validParams<IdealGasFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();
  params.addRequiredParam<Real>("gamma", "gamma value (cp/cv)");
  params.addRequiredParam<Real>("R", "Gas constant");
  params.addParam<Real>("mu", 0, "Dynamic viscosity, Pa.s");
  params.addParam<Real>("k", 0, "Thermal conductivity, W/(m-K)");
  params.addClassDescription("Fluid properties for an ideal gas");
  return params;
}

IdealGasFluidProperties::IdealGasFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _gamma(getParam<Real>("gamma")),
    _R(getParam<Real>("R")),
    _mu(getParam<Real>("mu")),
    _k(getParam<Real>("k"))
{
  _cp = _gamma * _R / (_gamma - 1.0);
  _cv = _cp / _gamma;
}

IdealGasFluidProperties::~IdealGasFluidProperties() {}

Real
IdealGasFluidProperties::p_from_v_e(Real v, Real e) const
{
  if (v == 0.0)
    throw MooseException(
        name() + ": Invalid value of specific volume detected (v = " + Moose::stringify(v) + ").");

  return (_gamma - 1.0) * e / v;
}

void
IdealGasFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
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

void
IdealGasFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

Real
IdealGasFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);

  const Real c2 = _gamma * _R * T;
  if (c2 < 0)
    mooseException(name() + ": Sound speed squared (gamma * R * T) is negative: c2 = " +
                   Moose::stringify(c2) + ".");

  return std::sqrt(c2);
}

void
IdealGasFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  c = std::sqrt(_gamma * _R * T);

  const Real dc_dT = 0.5 / c * _gamma * _R;
  dc_dv = dc_dT * dT_dv;
  dc_de = dc_dT * dT_de;
}

Real IdealGasFluidProperties::cp_from_v_e(Real, Real) const { return _cp; }

Real
IdealGasFluidProperties::cp() const
{
  return _cp;
}

Real IdealGasFluidProperties::cv_from_v_e(Real, Real) const { return _cv; }

Real
IdealGasFluidProperties::cv() const
{
  return _cv;
}

Real
IdealGasFluidProperties::gamma() const
{
  return _gamma;
}

Real IdealGasFluidProperties::mu_from_v_e(Real, Real) const { return _mu; }

Real IdealGasFluidProperties::k_from_v_e(Real, Real) const { return _k; }

Real
IdealGasFluidProperties::s_from_v_e(Real v, Real e) const
{
  const Real T = T_from_v_e(v, e);
  const Real p = p_from_v_e(v, e);
  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
    throw MooseException(name() + ": Negative argument in the ln() function.");
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
    throw MooseException(name() + ": Negative argument in the ln() function.");

  s = _cv * std::log(n);

  const Real dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p, _gamma - 1.0);
  const Real dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p, -_gamma);

  const Real dn_dv = dn_dT * dT_dv + dn_dp * dp_dv;
  const Real dn_de = dn_dT * dT_de + dn_dp * dp_de;

  ds_dv = _cv / n * dn_dv;
  ds_de = _cv / n * dn_de;
}

Real
IdealGasFluidProperties::s_from_p_T(Real p, Real T) const
{
  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
    throw MooseException(name() + ": Negative argument in the ln() function.");
  return _cv * std::log(n);
}

void
IdealGasFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  const Real n = std::pow(T, _gamma) / std::pow(p, _gamma - 1.0);
  if (n <= 0.0)
    throw MooseException(name() + ": Negative argument in the ln() function.");

  s = _cv * std::log(n);

  const Real dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p, _gamma - 1.0);
  const Real dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p, -_gamma);

  ds_dp = _cv / n * dn_dp;
  ds_dT = _cv / n * dn_dT;
}

Real
IdealGasFluidProperties::s_from_h_p(Real h, Real p) const
{
  const Real aux = p * std::pow(h / (_gamma * _cv), -_gamma / (_gamma - 1));
  if (aux <= 0.0)
    throw MooseException(name() + ": Non-positive argument in the ln() function.");
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
  if ((_gamma - 1.0) * p == 0.0)
    throw MooseException(name() +
                         ": Invalid gamma or pressure detected in rho_from_p_T(pressure = " +
                         Moose::stringify(p) + ", gamma = " + Moose::stringify(_gamma) + ")");

  return p / (_gamma - 1.0) / _cv / T;
}

void
IdealGasFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = 1.0 / (_gamma - 1.0) / _cv / T;
  drho_dT = -p / (_gamma - 1.0) / _cv / (T * T);
}

Real
IdealGasFluidProperties::e_from_p_rho(Real p, Real rho) const
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

Real
IdealGasFluidProperties::h_from_p_T(Real p, Real T) const
{
  Real rho = rho_from_p_T(p, T);
  Real e = T * _cv;
  return e + p / rho;
}

void
IdealGasFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  Real rho = rho_from_p_T(p, T);
  dh_dp = 0.0;
  dh_dT = _cv + p / rho / T;
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
  de_dp = 0.;
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
  return _R / R_universal;
}

Real
IdealGasFluidProperties::T_from_p_h(Real, Real h) const
{
  return h / _gamma / _cv;
}
