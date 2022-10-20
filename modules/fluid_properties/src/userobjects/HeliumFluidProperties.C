//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeliumFluidProperties.h"

registerMooseObject("FluidPropertiesApp", HeliumFluidProperties);

InputParameters
HeliumFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for helium");
  return params;
}

HeliumFluidProperties::HeliumFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _cv(3117.0), _cp(5195.0)
{
}

std::string
HeliumFluidProperties::fluidName() const
{
  return "helium";
}

ADReal
HeliumFluidProperties::e_from_p_rho(const ADReal & p, const ADReal & rho) const
{
  // Initial guess using ideal gas law
  ADReal e = p / (_cp / _cv - 1.) / rho;
  const ADReal v = 1. / rho;

  ADReal p_from_props, dp_dv, dp_de;
  const unsigned int max_its = 10;
  unsigned int it = 0;

  do
  {
    p_from_v_e(v, e, p_from_props, dp_dv, dp_de);
    const ADReal & jacobian = dp_de;
    const ADReal residual = p_from_props - p;

    if (std::abs(residual.value()) / p.value() < 1e-12)
      break;

    const ADReal delta_e = -residual / jacobian;
    e += delta_e;
  } while (++it < max_its);

  mooseAssert(it < max_its, "The iteration failed to converge");

  return e;
}

Real
HeliumFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return T / (48.14 * v - 0.4446 / std::pow(T, 0.2)) * 1.0e5;
}

ADReal
HeliumFluidProperties::p_from_v_e(const ADReal & v, const ADReal & e) const
{
  const ADReal T = T_from_v_e(v, e);
  return T / (48.14 * v - 0.4446 / std::pow(T, 0.2)) * 1.0e5;
}

void
HeliumFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);

  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  Real val = 48.14 * v - 0.4446 / std::pow(T, 0.2);
  Real dp_dT = 1.0e5 / val - 0.4446 * 0.2e5 * std::pow(T, -0.2) / (val * val);

  dp_dv = -48.14e5 * T / (val * val); // taking advantage of dT_dv = 0.0;
  dp_de = dp_dT * dT_de;
}

void
HeliumFluidProperties::p_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & p, DualReal & dp_dv, DualReal & dp_de) const
{
  p = p_from_v_e(v, e);

  DualReal T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  auto val = 48.14 * v - 0.4446 / std::pow(T, 0.2);
  auto dp_dT = 1.0e5 / val - 0.4446 * 0.2e5 * std::pow(T, -0.2) / (val * val);

  dp_dv = -48.14e5 * T / (val * val); // taking advantage of dT_dv = 0.0;
  dp_de = dp_dT * dT_de;
}

Real
HeliumFluidProperties::p_from_T_v(const Real T, const Real v) const
{
  // Formula taken from p_from_v_e method
  return T / (48.14 * v - 0.4446 / std::pow(T, 0.2)) * 1.0e5;
}

ADReal
HeliumFluidProperties::p_from_T_v(const ADReal & T, const ADReal & v) const
{
  // Formula taken from p_from_v_e method
  return T / (48.14 * v - 0.4446 / std::pow(T, 0.2)) * 1.0e5;
}

Real
HeliumFluidProperties::e_from_T_v(const Real T, const Real /*v*/) const
{
  // Formula taken from e_from_p_T method
  return _cv * T;
}

void
HeliumFluidProperties::e_from_T_v(
    const Real T, const Real v, Real & e, Real & de_dT, Real & de_dv) const
{
  e = e_from_T_v(T, v);
  de_dT = _cv;
  de_dv = 0;
}

ADReal
HeliumFluidProperties::e_from_T_v(const ADReal & T, const ADReal & /*v*/) const
{
  // Formula taken from e_from_p_T method
  return _cv * T;
}

void
HeliumFluidProperties::e_from_T_v(
    const ADReal & T, const ADReal & v, ADReal & e, ADReal & de_dT, ADReal & de_dv) const
{
  e = e_from_T_v(T, v);
  de_dT = _cv;
  de_dv = 0;
}

Real
HeliumFluidProperties::T_from_v_e(Real /*v*/, Real e) const
{
  return e / _cv;
}

ADReal
HeliumFluidProperties::T_from_v_e(const ADReal & /*v*/, const ADReal & e) const
{
  return e / _cv;
}

void
HeliumFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

void
HeliumFluidProperties::T_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & T, DualReal & dT_dv, DualReal & dT_de) const
{
  T = SinglePhaseFluidProperties::T_from_v_e(v, e);
  dT_dv = 0.0;
  dT_de = 1.0 / _cv;
}

Real
HeliumFluidProperties::T_from_p_h(Real /* p */, Real h) const
{
  return h / _cp;
}

Real
HeliumFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real p = p_from_v_e(v, e);
  Real T = T_from_v_e(v, e);

  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  Real c2 = -(p / rho / rho - _cv / drho_dT) / (_cv * drho_dp / drho_dT);
  return std::sqrt(c2);
}

void
HeliumFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  DualReal myv = v;
  Moose::derivInsert(myv.derivatives(), 0, 1);
  Moose::derivInsert(myv.derivatives(), 1, 0);
  DualReal mye = e;
  Moose::derivInsert(mye.derivatives(), 0, 0);
  Moose::derivInsert(mye.derivatives(), 1, 1);

  auto p = SinglePhaseFluidProperties::p_from_v_e(myv, mye);
  auto T = SinglePhaseFluidProperties::T_from_v_e(myv, mye);

  DualReal rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  auto cc = std::sqrt(-(p / rho / rho - _cv / drho_dT) / (_cv * drho_dp / drho_dT));
  c = cc.value();
  dc_dv = cc.derivatives()[0];
  dc_de = cc.derivatives()[1];
}

Real HeliumFluidProperties::cp_from_v_e(Real /*v*/, Real /*e*/) const { return _cp; }

void
HeliumFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_dv = 0.0;
  dcp_de = 0.0;
}

Real HeliumFluidProperties::cv_from_v_e(Real /*v*/, Real /*e*/) const { return _cv; }

void
HeliumFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = 0.0;
  dcv_de = 0.0;
}

Real
HeliumFluidProperties::mu_from_v_e(Real v, Real e) const
{
  return 3.674e-7 * std::pow(T_from_v_e(v, e), 0.7);
}

Real
HeliumFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real p_in_bar = p_from_v_e(v, e) * 1.0e-5;
  Real T = T_from_v_e(v, e);
  return 2.682e-3 * (1.0 + 1.123e-3 * p_in_bar) * std::pow(T, 0.71 * (1.0 - 2.0e-4 * p_in_bar));
}

Real
HeliumFluidProperties::beta_from_p_T(Real pressure, Real temperature) const
{
  Real rho;
  Real drho_dT;
  Real drho_dp;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  return -drho_dT / rho;
}

Real
HeliumFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  Real p_in_bar = pressure * 1.0e-5;
  return 48.14 * p_in_bar / (temperature + 0.4446 * p_in_bar / std::pow(temperature, 0.2));
}

void
HeliumFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  Real val = 1.0 / (temperature + 0.4446e-5 * pressure / std::pow(temperature, 0.2));
  drho_dp = 48.14e-5 * (val - 0.4446e-5 * pressure * val * val / std::pow(temperature, 0.2));
  drho_dT =
      -48.14e-5 * pressure * val * val * (1.0 - 0.08892e-5 * pressure / std::pow(temperature, 1.2));
}

void
HeliumFluidProperties::rho_from_p_T(const DualReal & pressure,
                                    const DualReal & temperature,
                                    DualReal & rho,
                                    DualReal & drho_dp,
                                    DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  auto val = 1.0 / (temperature + 0.4446e-5 * pressure / std::pow(temperature, 0.2));
  drho_dp = 48.14e-5 * (val - 0.4446e-5 * pressure * val * val / std::pow(temperature, 0.2));
  drho_dT =
      -48.14e-5 * pressure * val * val * (1.0 - 0.08892e-5 * pressure / std::pow(temperature, 1.2));
}

Real
HeliumFluidProperties::e_from_p_T(Real /*pressure*/, Real temperature) const
{
  return _cv * temperature;
}

void
HeliumFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(pressure, temperature);
  de_dp = 0.0;
  de_dT = _cv;
}

Real
HeliumFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  return _cp * temperature;
}

void
HeliumFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0.0;
  dh_dT = _cp;
}

Real
HeliumFluidProperties::molarMass() const
{
  return 4.002602e-3;
}

Real HeliumFluidProperties::cp_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cp;
}

void
HeliumFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0.0;
  dcp_dT = 0.0;
}

Real HeliumFluidProperties::cv_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cv;
}

void
HeliumFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 0.0;
}

Real
HeliumFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 3.674e-7 * std::pow(temperature, 0.7);
}

void
HeliumFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;
  dmu_dT = 3.674e-7 * 0.7 * std::pow(temperature, -0.3);
}

Real
HeliumFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  return 2.682e-3 * (1.0 + 1.123e-8 * pressure) *
         std::pow(temperature, 0.71 * (1.0 - 2.0e-9 * pressure));
}

void
HeliumFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(pressure, temperature);

  Real term = 1.0 + 1.123e-8 * pressure;
  Real exp = 0.71 * (1.0 - 2.0e-9 * pressure);

  dk_dp = 2.682e-3 * (term * 0.71 * (-2.0e-9) * std::log(temperature) * std::pow(temperature, exp) +
                      std::pow(temperature, exp) * 1.123e-8);

  dk_dT = 2.682e-3 * term * exp * std::pow(temperature, exp - 1.0);
}
