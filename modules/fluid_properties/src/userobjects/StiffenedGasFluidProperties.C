/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StiffenedGasFluidProperties.h"

template <>
InputParameters
validParams<StiffenedGasFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();

  params.addRequiredParam<Real>("gamma", "Heat capacity ratio");
  params.addRequiredParam<Real>("cv", "Constant volume specific heat");
  params.addRequiredParam<Real>("q", "");
  params.addRequiredParam<Real>("p_inf", "");
  params.addParam<Real>("q_prime", 0, "Parameter");

  params.addParam<Real>("mu", 1.e-3, "Dynamic viscosity, Pa.s");
  params.addParam<Real>("k", 0.6, "Thermal conductivity, W/(m-K)");

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
StiffenedGasFluidProperties::pressure(Real v, Real u) const
{
  return (_gamma - 1) * (u - _q) / v - _gamma * _p_inf;
}

Real
StiffenedGasFluidProperties::temperature(Real v, Real u) const
{
  return (1 / _cv) * (u - _q - _p_inf * v);
}

Real
StiffenedGasFluidProperties::c(Real v, Real u) const
{
  return std::sqrt(_gamma * (this->pressure(v, u) + _p_inf) * v);
}

Real StiffenedGasFluidProperties::cp(Real, Real) const { return _cp; }

Real StiffenedGasFluidProperties::cv(Real, Real) const { return _cv; }

Real StiffenedGasFluidProperties::mu(Real, Real) const { return _mu; }

Real StiffenedGasFluidProperties::k(Real, Real) const { return _k; }

Real
StiffenedGasFluidProperties::s(Real v, Real u) const
{
  Real T = this->temperature(v, u);
  Real p = this->pressure(v, u);
  Real n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1);
  if (n <= 0)
    mooseError(name(), ": Negative argument in the ln() function.");
  return _cv * std::log(n) + _q_prime;
}

void
StiffenedGasFluidProperties::dp_duv(
    Real v, Real u, Real & dp_dv, Real & dp_du, Real & dT_dv, Real & dT_du) const
{
  dp_dv = -(_gamma - 1) * (u - _q) / v / v;
  dp_du = (_gamma - 1) / v;
  dT_dv = -_p_inf / _cv;
  dT_du = 1 / _cv;
}

void
StiffenedGasFluidProperties::rho_e_ps(Real pressure, Real entropy, Real & rho, Real & e) const
{
  Real a = (entropy - _q_prime + _cv * std::log(std::pow(pressure + _p_inf, _gamma - 1))) / _cv;
  Real T = std::pow(std::exp(a), 1 / _gamma);
  rho = this->rho(pressure, T);
  e = this->e(pressure, rho);
}

void
StiffenedGasFluidProperties::rho_e_dps(Real pressure,
                                       Real entropy,
                                       Real & rho,
                                       Real & drho_dp,
                                       Real & drho_ds,
                                       Real & e,
                                       Real & de_dp,
                                       Real & de_ds) const
{
  // compute rho(p, T(p,s)) and e(p, rho(p, T(p,s)))
  this->rho_e_ps(pressure, entropy, rho, e);

  // compute temperature
  const Real aux =
      (entropy - _q_prime + _cv * std::log(std::pow(pressure + _p_inf, _gamma - 1))) / _cv;
  const Real T = std::pow(std::exp(aux), 1 / _gamma);

  // dT/dp
  const Real dT_dp = 1.0 / _gamma * std::pow(std::exp(aux), 1.0 / _gamma - 1.0) * std::exp(aux) /
                     std::pow(pressure + _p_inf, _gamma - 1.0) * (_gamma - 1.0) *
                     std::pow(pressure + _p_inf, _gamma - 2.0);

  // dT/ds
  const Real dT_ds =
      1.0 / _gamma * std::pow(std::exp(aux), 1.0 / _gamma - 1.0) * std::exp(aux) / _cv;

  // Drho/Dp = d/dp[rho(p, T(p,s))] = drho/dp + drho/dT * dT/dp
  Real drho_dp_partial, drho_dT;
  rho_dpT(pressure, T, rho, drho_dp_partial, drho_dT);
  drho_dp = drho_dp_partial + drho_dT * dT_dp;

  // Drho/Ds = d/ds[rho(p, T(p,s))] = drho/dT * dT/ds
  drho_ds = drho_dT * dT_ds;

  // De/Dp = d/dp[e(p, rho(p, T(p,s)))] = de/dp + de/drho * Drho/Dp
  const Real de_dp_partial = 1.0 / ((_gamma - 1.0) * rho);
  const Real de_drho = -(pressure + _gamma * _p_inf) / ((_gamma - 1.0) * rho * rho);
  de_dp = de_dp_partial + de_drho * drho_dp;

  // De/Ds = d/ds[e(p, rho(p, T(p,s)))] = de/drho * Drho/Ds
  de_ds = de_drho * drho_ds;
}

Real
StiffenedGasFluidProperties::beta(Real pressure, Real temperature) const
{
  // The volumetric thermal expansion coefficient is defined as
  //   1/v dv/dT)_p
  // It is the fractional change rate of volume with respect to temperature change
  // at constant pressure. Here it is coded as
  //   - 1/rho drho/dT)_p
  // using chain rule with v = v(rho)

  Real rho, drho_dp, drho_dT;
  rho_dpT(pressure, temperature, rho, drho_dp, drho_dT);
  return -drho_dT / rho;
}

void
StiffenedGasFluidProperties::rho_e(Real pressure, Real temperature, Real & rho, Real & e) const
{
  rho = this->rho(pressure, temperature);
  e = this->e(pressure, rho);
}

Real
StiffenedGasFluidProperties::rho(Real pressure, Real temperature) const
{
  mooseAssert(((_gamma - 1) * _cv * temperature) != 0.0,
              "Invalid gamma or cv or temperature detected!");
  return (pressure + _p_inf) / ((_gamma - 1) * _cv * temperature);
}

void
StiffenedGasFluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  mooseAssert(((_gamma - 1) * _cv * temperature) != 0.0,
              "Invalid gamma or cv or temperature detected!");
  rho = (pressure + _p_inf) / ((_gamma - 1) * _cv * temperature);
  drho_dp = 1. / ((_gamma - 1) * _cv * temperature);
  drho_dT = -(pressure + _p_inf) / ((_gamma - 1) * _cv * temperature * temperature);
}

void
StiffenedGasFluidProperties::e_dpT(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": e_dpT() not implemented.");
}

Real
StiffenedGasFluidProperties::e(Real pressure, Real rho) const
{
  mooseAssert((_gamma - 1) * rho != 0., "Invalid gamma or density detected!");
  return (pressure + _gamma * _p_inf) / ((_gamma - 1) * rho) + _q;
}

void
StiffenedGasFluidProperties::e_dprho(
    Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  mooseAssert((_gamma - 1) * rho != 0., "Invalid gamma or density detected!");
  e = this->e(pressure, rho);
  de_dp = 1. / ((_gamma - 1) * rho);
  de_drho = -(pressure + _gamma * _p_inf) / ((_gamma - 1) * rho * rho);
}

Real
StiffenedGasFluidProperties::h(Real, Real temperature) const
{
  return _gamma * _cv * temperature + _q;
}

void
StiffenedGasFluidProperties::h_dpT(
    Real, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = _gamma * _cv * temperature + _q;
  dh_dp = 0;
  dh_dT = _gamma * _cv;
}

Real
StiffenedGasFluidProperties::p_from_h_s(Real h, Real s) const
{
  return std::pow((h - _q) / (_gamma * _cv), _gamma / (_gamma - 1)) *
             std::exp((_q_prime - s) / ((_gamma - 1) * _cv)) -
         _p_inf;
}

Real
StiffenedGasFluidProperties::dpdh_from_h_s(Real h, Real s) const
{
  return _gamma / (_gamma - 1.0) / (_gamma * _cv) *
         std::pow((h - _q) / (_gamma * _cv), 1.0 / (_gamma - 1.0)) *
         std::exp((_q_prime - s) / ((_gamma - 1.0) * _cv));
}

Real StiffenedGasFluidProperties::gamma(Real, Real) const { return _gamma; }

Real
StiffenedGasFluidProperties::c2_from_p_rho(Real pressure, Real rho) const
{
  return _gamma * (pressure + _p_inf) / rho;
}
