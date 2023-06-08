//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlinakFluidProperties.h"

registerMooseObject("FluidPropertiesApp", FlinakFluidProperties);

InputParameters
FlinakFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addRangeCheckedParam<Real>(
      "drho_dp",
      1.7324E-7,
      "drho_dp > 0.0",
      "derivative of density with respect to pressure (at constant temperature)");
  params.addClassDescription("Fluid properties for flinak");
  return params;
}

FlinakFluidProperties::FlinakFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _drho_dp(getParam<Real>("drho_dp")),
    _drho_dT(-0.73),
    _p_atm(101325.0),
    _cp(2010.0),
    _c0(2729.0),
    _dp_dT_at_constant_v(-_drho_dT / _drho_dp)
{
}

std::string
FlinakFluidProperties::fluidName() const
{
  return "flinak";
}

Real
FlinakFluidProperties::molarMass() const
{
  return 41.291077435E-3;
}

Real
FlinakFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return (1.0 / v - _drho_dT * temperature - _c0) / _drho_dp + _p_atm;
}

void
FlinakFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);

  // chain rule, (dp_de)_v = (dp_dT)_v * (dT_de)_v
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  dp_de = _dp_dT_at_constant_v * dT_de;

  // cyclic relation, (dP_dv)_e = - (dp_de)_v * (de_dv)_p
  Real cp = cp_from_v_e(v, e);
  Real dT_dv_at_constant_p = -1.0 / (_drho_dT * v * v);
  Real de_dv_at_constant_p = cp * dT_dv_at_constant_p - p;
  dp_dv = -dp_de * de_dv_at_constant_p;
}

void
FlinakFluidProperties::p_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & p, DualReal & dp_dv, DualReal & dp_de) const
{
  p = SinglePhaseFluidProperties::p_from_v_e(v, e);

  // chain rule, (dp_de)_v = (dp_dT)_v * (dT_de)_v
  DualReal T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  dp_de = _dp_dT_at_constant_v * dT_de;

  // cyclic relation, (dP_dv)_e = - (dp_de)_v * (de_dv)_p
  auto cp = SinglePhaseFluidProperties::cp_from_v_e(v, e);
  auto dT_dv_at_constant_p = -1.0 / (_drho_dT * v * v);
  auto de_dv_at_constant_p = cp * dT_dv_at_constant_p - p;
  dp_dv = -dp_de * de_dv_at_constant_p;
}

Real
FlinakFluidProperties::T_from_v_e(Real v, Real e) const
{
  // We need to write these in a somewhat strange manner to ensure that pressure
  // and temperature do not depend implicitly on each other, causing a circular
  // logic problem. Substituting the definition for pressure based on the
  // rho * (h - e) = P, where h = Cp * T into the density correlation for flibe,
  // we can rearrange and get temperature in terms of only v and e

  // p = (Cp * T - e) / v
  // T = (1 / v - drho_dp * [p                - p_atm] + _c0) / drho_dT
  //   = (1 / v - drho_dp * [(Cp * T - e) / v - p_atm] + _c0) / drho_dT
  //   = (1 + drho_dp * e + p_atm * v * drho_dp - _c0 * v) / (drho_dT * v + drho_dp * Cp)

  Real cp = cp_from_v_e(v, e);
  Real numerator = 1.0 + _drho_dp * (e + _p_atm * v) - _c0 * v;
  Real denominator = _drho_dT * v + _drho_dp * cp;
  return numerator / denominator;
}

void
FlinakFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);

  // reciprocity relation based on the definition of cv
  Real cv = cv_from_v_e(v, e);
  dT_de = 1.0 / cv;

  // cyclic relation, (dT_dv)_e = -(dT_de)_v * (de_dv)_T
  Real p = p_from_v_e(v, e);
  Real dp_dv_at_constant_T = -1.0 / (_drho_dp * v * v);
  Real de_dv_at_constant_T = -(p + v * dp_dv_at_constant_T);
  dT_dv = -dT_de * de_dv_at_constant_T;
}

void
FlinakFluidProperties::T_from_v_e(
    const DualReal & v, const DualReal & e, DualReal & T, DualReal & dT_dv, DualReal & dT_de) const
{
  T = SinglePhaseFluidProperties::T_from_v_e(v, e);

  // reciprocity relation based on the definition of cv
  auto cv = SinglePhaseFluidProperties::cv_from_v_e(v, e);
  dT_de = 1.0 / cv;

  // cyclic relation, (dT_dv)_e = -(dT_de)_v * (de_dv)_T
  auto p = SinglePhaseFluidProperties::p_from_v_e(v, e);
  auto dp_dv_at_constant_T = -1.0 / (_drho_dp * v * v);
  auto de_dv_at_constant_T = -(p + v * dp_dv_at_constant_T);
  dT_dv = -dT_de * de_dv_at_constant_T;
}

Real
FlinakFluidProperties::T_from_p_h(Real /* p */, Real h) const
{
  return h / _cp;
}

Real
FlinakFluidProperties::T_from_p_rho(Real p, Real rho) const
{
  Real temperature = (rho - (p - _p_atm) * _drho_dp - _c0) / _drho_dT;
  return temperature;
}

Real FlinakFluidProperties::cp_from_v_e(Real /*v*/, Real /*e*/) const { return _cp; }

void
FlinakFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_dv = 0.0;
  dcp_de = 0.0;
}

Real
FlinakFluidProperties::cv_from_v_e(Real v, Real e) const
{
  // definition of Cv by replacing e by h + p * v
  Real cp = cp_from_v_e(v, e);
  return cp - _dp_dT_at_constant_v * v;
}

void
FlinakFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = -_dp_dT_at_constant_v;
  dcv_de = 0.0;
}

void
FlinakFluidProperties::cv_from_v_e(const DualReal & v,
                                   const DualReal & e,
                                   DualReal & cv,
                                   DualReal & dcv_dv,
                                   DualReal & dcv_de) const
{
  cv = SinglePhaseFluidProperties::cv_from_v_e(v, e);
  dcv_dv = -_dp_dT_at_constant_v;
  dcv_de = 0.0;
}

Real
FlinakFluidProperties::mu_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 4.0e-5 * std::exp(4170.0 / temperature);
}

Real
FlinakFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 5.0e-4 * temperature + 0.43;
}

Real
FlinakFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  return _drho_dT * temperature + _drho_dp * (pressure - _p_atm) + _c0;
}

void
FlinakFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = _drho_dp;
  drho_dT = _drho_dT;
}

void
FlinakFluidProperties::rho_from_p_T(const DualReal & pressure,
                                    const DualReal & temperature,
                                    DualReal & rho,
                                    DualReal & drho_dp,
                                    DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  drho_dp = _drho_dp;
  drho_dT = _drho_dT;
}

DualReal
FlinakFluidProperties::v_from_p_T(const DualReal & pressure, const DualReal & temperature) const
{
  return 1.0 / (_drho_dT * temperature + _drho_dp * (pressure - _p_atm) + _c0);
}

Real
FlinakFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  return 1.0 / (_drho_dT * temperature + _drho_dp * (pressure - _p_atm) + _c0);
}

void
FlinakFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(pressure, temperature);
  dv_dp = -v * v * _drho_dp;
  dv_dT = -v * v * _drho_dT;
}

Real
FlinakFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  // definition of h for constant Cp
  Real cp = cp_from_v_e(0.0 /* dummy */, 0.0 /* dummy */);
  return cp * temperature;
}

void
FlinakFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  Real cp = cp_from_v_e(0.0 /* dummy */, 0.0 /* dummy */);

  dh_dp = 0.0;
  dh_dT = cp;
}

Real
FlinakFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  // definition of h = e + p * v
  Real v = v_from_p_T(pressure, temperature);
  Real cp = cp_from_v_e(v, 0.0 /* dummy */);
  return cp * temperature - pressure * v;
}

void
FlinakFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(pressure, temperature);

  Real v, dv_dp, dv_dT;
  v_from_p_T(pressure, temperature, v, dv_dp, dv_dT);

  // definition of e = h - p * v
  de_dp = -pressure * dv_dp - v;

  // definition of e = h - p * v
  Real cp = cp_from_v_e(v, e);
  de_dT = cp - pressure * dv_dT;
}

Real
FlinakFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_T(p, T_from_p_rho(p, rho));
}

Real
FlinakFluidProperties::beta_from_p_T(Real pressure, Real temperature) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  return -drho_dT / rho;
}

Real FlinakFluidProperties::cp_from_p_T(Real /*pressure*/, Real /*temperature*/) const
{
  return _cp;
}

void
FlinakFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0.0;
  dcp_dT = 0.0;
}

Real
FlinakFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  Real v = v_from_p_T(pressure, temperature);
  Real e = e_from_p_T(pressure, temperature);
  return cv_from_v_e(v, e);
}

void
FlinakFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 0.0;
}

Real
FlinakFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 4.0e-5 * std::exp(4170.0 / temperature);
}

void
FlinakFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;
  dmu_dT = -4.0e-5 * std::exp(4170.0 / temperature) * 4170.0 / (temperature * temperature);
}

Real
FlinakFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 5.0e-4 * temperature + 0.43;
}

void
FlinakFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k_from_p_T(pressure, temperature);
  dk_dp = 0.0;
  dk_dT = 5.0e-4;
}
