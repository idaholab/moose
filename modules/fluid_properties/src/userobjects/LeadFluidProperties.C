//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeadFluidProperties.h"

registerMooseObject("FluidPropertiesApp", LeadFluidProperties);

InputParameters
LeadFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addParam<Real>("T_mo", 600.6, "Melting Point of Lead");
  // params.addRangeCheckedParam<Real>(
  //     "drho_dp",
  //     1.7324E-7,
  //     "drho_dp > 0.0",
  //     "derivative of density with respect to pressure (at constant temperature)");
  // params.addClassDescription("Fluid properties for Lead");
  return params;
}

LeadFluidProperties::LeadFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _T_mo(getParam<Real>("T_mo"))
// _drho_dp(getParam<Real>("drho_dp")),
// _drho_dT(-0.4884),
// _p_atm(101325.0),
// _cp(2416.0),
// _c0(2413.0),
// _dp_dT_at_constant_v(-_drho_dT / _drho_dp)
{
}

std::string
LeadFluidProperties::fluidName() const
{
  return "Lead";
}

Real
LeadFluidProperties::molarMass() const
{
  return 207.2;
}

Real
LeadFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real h = h_from_v_e(v, e);
  return (h - e) / v;
}

void
LeadFluidProperties::p_from_v_e(Real v, Real e, Real & /*p*/, Real & dp_dv, Real & dp_de) const
{
  Real h, dh_dv, dh_de;
  h_from_v_e(v, e, h, dh_dv, dh_de);
  dp_dv = (v * dh_dv - h + e) / v / v;
  dp_de = (dh_de - 1) / v;
}

Real
LeadFluidProperties::mu_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 4.55e-4 * std::exp(1069 / temperature);
}

void
LeadFluidProperties::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  mu = mu_from_v_e(v, e);
  dmu_dv = dT_dv * -1069 * 4.55e-4 * exp(1069 / temperature) / temperature / temperature;
  dmu_de = dT_de * -1069 * 4.55e-4 * exp(1069 / temperature) / temperature / temperature;
}

Real
LeadFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 0.011 * temperature + 9.2;
}

void
LeadFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  k = k_from_v_e(v, e);
  dk_dv = 0.011 * dT_dv;
  dk_de = 0.011 * dT_de;
}

Real
LeadFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 11441 - 1.2795 * temperature;
}

void
LeadFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  drho_dT = -1.2795;
}

void
LeadFluidProperties::rho_from_p_T(const DualReal & pressure,
                                  const DualReal & temperature,
                                  DualReal & rho,
                                  DualReal & drho_dp,
                                  DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  drho_dT = -1.2795;
}

Real
LeadFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  return 1.0 / rho_from_p_T(pressure, temperature);
}

void
LeadFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(pressure, temperature);
  dv_dp = 0;
  dv_dT = -1.2795 / MathUtils::pow(11441 - 1.2795 * temperature, 2);
}

Real
LeadFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 176.2 * (temperature - _T_mo) - 2.4615e-2 * (temperature * temperature - _T_mo * _T_mo) +
         5.147e-6 * (temperature * temperature * temperature - _T_mo * _T_mo * _T_mo) +
         1.524e+6 * (1 / temperature - 1 / _T_mo);
}

void
LeadFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0;
  dh_dT = 176.2 + 2 * 2.4615e-2 * temperature + 3 * 5.147e-6 * temperature * temperature -
          1.524e+6 / temperature / temperature;
}

Real
LeadFluidProperties::h_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 176.2 * (temperature - _T_mo) - 2.4615e-2 * (temperature * temperature - _T_mo * _T_mo) +
         5.147e-6 * (temperature * temperature * temperature - _T_mo * _T_mo * _T_mo) +
         1.524e+6 * (1 / temperature - 1 / _T_mo);
}

void
LeadFluidProperties::h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  h = h_from_v_e(v, e);
  dh_dv = 176.2 * dT_dv + 2 * dT_dv * 2.4615e-2 * temperature +
          3 * 5.147e-6 * temperature * temperature * dT_dv -
          1.524e+6 / temperature / temperature / dT_dv;
  dh_de = 176.2 * dT_de + 2 * dT_de * 2.4615e-2 * temperature +
          3 * 5.147e-6 * temperature * temperature * dT_de -
          1.524e+6 / temperature / temperature / dT_de;
}

Real
LeadFluidProperties::e_from_p_T(Real p, Real T) const
{
  // definition of h = e + p * v
  Real v = v_from_p_T(p, T);
  Real h = h_from_p_T(p, T);
  return h - p * v;
}

void
LeadFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  Real dh_dp, dv_dp, dh_dT, dv_dT, v, h;
  h_from_p_T(p, T, h, dh_dp, dh_dT);
  v_from_p_T(p, T, v, dv_dp, dv_dT);
  e = h_from_p_T(v, e);
  de_dp = dh_dp - v - dv_dp * p;
  de_dT = dh_dT - dv_dT * p;
}

Real
LeadFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_T(p, T_from_p_rho(p, rho));
}

Real
LeadFluidProperties::T_from_p_rho(Real /*pressure*/, Real rho) const
{
  return (rho - 11441) / -1.2795;
}

void
LeadFluidProperties::T_from_p_rho(
    Real pressure, Real rho, Real & temperature, Real & dT_dp, Real & dT_drho) const
{
  temperature = T_from_p_rho(pressure, rho);
  dT_dp = 0;
  dT_drho = 1 / -1.2795;
}

Real
LeadFluidProperties::T_from_v_e(Real v, Real /*e*/) const
{
  return (1 / v - 11441) * -1 / 1.2795;
}

void
LeadFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  // Returning straight derivatives since no pressure dependence
  dT_de = 0;
  dT_dv = 1 / v / v / 1.2795;
}

Real
LeadFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 176.2 - 4.923e-2 * temperature + 1.544e-5 * temperature * temperature -
         1.524e+6 / temperature / temperature;
}

void
LeadFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  dcp_dT = -4.923e-2 + 2 * 1.55e-5 * temperature +
           2 * 1.524e+6 / temperature / temperature / temperature;
}

Real
LeadFluidProperties::cp_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 176.2 - 4.923e-2 * temperature + 1.544e-5 * temperature * temperature -
         1.524e+6 / temperature / temperature;
}

void
LeadFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  cp = cp_from_v_e(v, e);
  dcp_dv = -4.923e-2 * dT_dv + 2 * dT_dv * 1.554e-5 * temperature +
           2 * dT_dv * 1.524e+6 / temperature / temperature / temperature;

  dcp_de = -4.923e-2 * dT_de + 2 * dT_de * 1.554e-5 * temperature +
           2 * dT_de * 1.524e+6 / temperature / temperature / temperature;
}

Real
LeadFluidProperties::cv_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e);
}

void
LeadFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  Real cp, dcp_dv, dcp_de;
  cp_from_v_e(v, e, cp, dcp_dv, dcp_de);
  cv = cv_from_v_e(v, e);
  dcv_dv = dcp_dv;
  dcv_de = dcp_de;
}

Real
LeadFluidProperties::cv_from_p_T(Real p, Real T) const
{
  return cp_from_p_T(p, T);
}

void
LeadFluidProperties::cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  Real cp, dcp_dp, dcp_dT;
  cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
  cv = cv_from_p_T(p, T);
  dcv_dp = dcp_dp;
  dcv_dT = dcp_dT;
}

Real
LeadFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 4.55e-4 * std::exp(1069 / temperature);
}

void
LeadFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(pressure, temperature);
  dmu_dp = 0;
  dmu_dT = -1069 * 4.55e-4 * std::exp(1069 / temperature) / temperature / temperature;
}

Real
LeadFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 0.011 * temperature + 9.2;
}

void
LeadFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(pressure, temperature);
  dk_dp = 0;
  dk_dT = 0.011;
}
