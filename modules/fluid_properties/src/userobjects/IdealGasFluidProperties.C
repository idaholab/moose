/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "IdealGasFluidProperties.h"

template <>
InputParameters
validParams<IdealGasFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();
  params.addRequiredParam<Real>("gamma", "gamma value (cp/cv)");
  params.addRequiredParam<Real>("R", "Gas constant");

  params.addParam<Real>("beta", 0, "Coefficient of thermal expansion");
  params.addParam<Real>("mu", 0, "Dynamic viscosity, Pa.s");
  params.addParam<Real>("k", 0, "Thermal conductivity, W/(m-K)");

  return params;
}

IdealGasFluidProperties::IdealGasFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _gamma(getParam<Real>("gamma")),
    _R(getParam<Real>("R")),

    _beta(getParam<Real>("beta")),
    _mu(getParam<Real>("mu")),
    _k(getParam<Real>("k"))
{
  _cp = _gamma * _R / (_gamma - 1);
  _cv = _cp / _gamma;
}

IdealGasFluidProperties::~IdealGasFluidProperties() {}

Real
IdealGasFluidProperties::pressure(Real v, Real u) const
{
  if (v == 0.0)
    mooseError(name(), ": Invalid value of specific volume detected (v = ", v, ").");

  // The std::max function serves as a hard limiter, which will guarantee non-negative pressure
  // when resolving strongly nonlinear waves
  return std::max(1e-8, (_gamma - 1.) * u / v);
}

Real
IdealGasFluidProperties::temperature(Real /*v*/, Real u) const
{
  return u / _cv;
}

Real
IdealGasFluidProperties::c(Real v, Real u) const
{
  Real temp = temperature(v, u);
  // The std::max function serves as a hard limiter, which will guarantee non-negative speed of
  // sound
  // when resolving strongly nonlinear waves
  return std::sqrt(std::max(1e-8, _gamma * _R * temp));
}

Real IdealGasFluidProperties::cp(Real, Real) const { return _cp; }

Real IdealGasFluidProperties::cv(Real, Real) const { return _cv; }

Real IdealGasFluidProperties::gamma(Real, Real) const { return _gamma; }

Real IdealGasFluidProperties::mu(Real, Real) const { return _mu; }

Real IdealGasFluidProperties::k(Real, Real) const { return _k; }

Real IdealGasFluidProperties::s(Real, Real) const
{
  mooseError(name(), ": s() not implemented.");
  return 0;
}

void
IdealGasFluidProperties::dp_duv(
    Real v, Real u, Real & dp_dv, Real & dp_du, Real & dT_dv, Real & dT_du) const
{
  dp_dv = -(_gamma - 1) * u / v / v;
  dp_du = (_gamma - 1) / v;
  dT_dv = 0;
  dT_du = 1 / _cv;
}

void
IdealGasFluidProperties::rho_e_ps(Real, Real, Real &, Real &) const
{
  mooseError(name(), ": rho_e_ps() not implemented.");
}

void
IdealGasFluidProperties::rho_e_dps(Real, Real, Real &, Real &, Real &, Real &, Real &, Real &) const
{
  mooseError(name(), ": rho_e_dps() not implemented.");
}

Real IdealGasFluidProperties::beta(Real, Real) const { return _beta; }

void
IdealGasFluidProperties::rho_e(Real pressure, Real temperature, Real & rho, Real & e) const
{
  rho = this->rho(pressure, temperature);
  e = temperature * _cv;
}

Real
IdealGasFluidProperties::rho(Real pressure, Real temperature) const
{
  if ((_gamma - 1) * pressure == 0.)
    mooseError(name(),
               ": Invalid gamma or pressure detected in rho(pressure = ",
               pressure,
               ", gamma = ",
               _gamma,
               ")");

  return pressure / (_gamma - 1.0) / _cv / temperature;
}

void
IdealGasFluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  Real temp2 = temperature * temperature;
  rho = pressure / (_gamma - 1.0) / _cv / temperature;
  drho_dp = 1 / (_gamma - 1.0) / _cv / temperature;
  drho_dT = -pressure / (_gamma - 1.0) / _cv / temp2;
}

void
IdealGasFluidProperties::e_dpT(Real, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = temperature * _cv;
  de_dp = 0;
  de_dT = _cv;
}

Real
IdealGasFluidProperties::e(Real pressure, Real rho) const
{
  return pressure / (_gamma - 1) / rho;
}

void
IdealGasFluidProperties::e_dprho(
    Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e = this->e(pressure, rho);
  de_dp = 1 / (_gamma - 1) / rho;
  de_drho = -pressure / (_gamma - 1) / rho / rho;
}

Real
IdealGasFluidProperties::h(Real pressure, Real temperature) const
{
  Real rho = this->rho(pressure, temperature);
  Real e = temperature * _cv;
  return e + pressure / rho;
}

void
IdealGasFluidProperties::h_dpT(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = this->h(pressure, temperature);
  Real rho = this->rho(pressure, temperature);
  dh_dp = 0;
  dh_dT = _cv + pressure / rho / temperature;
}

Real IdealGasFluidProperties::p_from_h_s(Real /*h*/, Real /*s*/) const
{
  mooseError(name(), ": p_from_h_s() not implemented.");
  return 0;
}

Real IdealGasFluidProperties::dpdh_from_h_s(Real /*h*/, Real /*s*/) const
{
  mooseError(name(), ": dpdh_from_h_s() not implemented.");
  return 0;
}
