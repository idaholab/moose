//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeadBismuthFluidProperties.h"

registerMooseObject("FluidPropertiesApp", LeadBismuthFluidProperties);

InputParameters
LeadBismuthFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addParam<Real>("T_mo", 398, "Melting Point of LeadBismuth (K)");
  params.addClassDescription("Fluid properties for LeadBismuth 2LiF-BeF2");

  return params;
}

LeadBismuthFluidProperties::LeadBismuthFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _T_mo(getParam<Real>("T_mo"))
{
}

std::string
LeadBismuthFluidProperties::fluidName() const
{
  return "LeadBismuth";
}

Real
LeadBismuthFluidProperties::molarMass() const
{
  return 233.99;
}

Real
LeadBismuthFluidProperties::bulk_modulus_from_p_T(Real /*p*/, Real T) const
{
  return (38.02 - 1.296e-2 * T + 1.32e-6 * T * T) * MathUtils::pow(10, 9);
}

Real
LeadBismuthFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 1855 - 0.212 * T;
}

DualReal
LeadBismuthFluidProperties::c_from_v_e(const DualReal & v, const DualReal & e) const
{
  DualReal T = SinglePhaseFluidProperties::T_from_v_e(v, e);
  return 1855 - 0.212 * T;
}

Real
LeadBismuthFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real h = h_from_v_e(v, e);
  return (h - e) / v;
}

void
LeadBismuthFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  Real h, dh_dv, dh_de;
  h_from_v_e(v, e, h, dh_dv, dh_de);
  dp_dv = (v * dh_dv - h + e) / v / v;
  dp_de = (dh_de - 1) / v;
}

Real
LeadBismuthFluidProperties::T_from_v_e(Real v, Real /*e*/) const
{
  return (1 / v - 11065) / -1.293;
}

void
LeadBismuthFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_de = 0;
  dT_dv = 1 / v / v / 1.293;
}

Real
LeadBismuthFluidProperties::cp_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 164.8 - 3.94e-2 * temperature + 1.25e-5 * temperature * temperature -
         4.56e+5 / temperature / temperature;
}

void
LeadBismuthFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  cp = cp_from_v_e(v, e);
  dcp_dv = -3.94e-2 * dT_dv + 2 * dT_dv * 1.25e-5 * temperature +
           2 * dT_dv * 4.56e+5 / temperature / temperature / temperature;

  dcp_de = -3.94e-2 * dT_de + 2 * dT_de * 1.25e-5 * temperature +
           2 * dT_de * 4.56e+5 / temperature / temperature / temperature;
}
Real
LeadBismuthFluidProperties::cv_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e);
}

void
LeadBismuthFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  Real cp, dcp_dv, dcp_de;
  cp_from_v_e(v, e, cp, dcp_dv, dcp_de);
  cv = cv_from_v_e(v, e);
  dcv_dv = dcp_dv;
  dcv_de = dcp_de;
}

Real
LeadBismuthFluidProperties::cv_from_p_T(Real p, Real T) const
{
  return cp_from_p_T(p, T);
}

void
LeadBismuthFluidProperties::cv_from_p_T(
    Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  Real cp, dcp_dp, dcp_dT;
  cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
  cv = cv_from_p_T(p, T);
  dcv_dp = dcp_dp;
  dcv_dT = dcp_dT;
}

Real
LeadBismuthFluidProperties::mu_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 4.94e-4 * std::exp(754.1 / temperature);
}

void
LeadBismuthFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  mu = mu_from_v_e(v, e);
  dmu_dv = dT_dv * -754.1 * 4.94e-4 * exp(754.1 / temperature) / temperature / temperature;
  dmu_de = dT_de * -754.1 * 4.94e-4 * exp(754.1 / temperature) / temperature / temperature;
}

Real
LeadBismuthFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 3.284 + 1.617e-2 * temperature - 2.305e-6 * temperature * temperature;
}

void
LeadBismuthFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  k = k_from_v_e(v, e);
  dk_dv = 1.617e-2 * dT_dv - 2 * 2.305e-6 * dT_dv * temperature;
  dk_de = 1.617e-2 * dT_de - 2 * 2.305e-6 * dT_de * temperature;
}

Real
LeadBismuthFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 11065 - 1.293 * temperature;
}

void
LeadBismuthFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  drho_dT = -1.293;
}

void
LeadBismuthFluidProperties::rho_from_p_T(const DualReal & pressure,
                                         const DualReal & temperature,
                                         DualReal & rho,
                                         DualReal & drho_dp,
                                         DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  drho_dT = -1.293;
}

Real
LeadBismuthFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  return 1.0 / rho_from_p_T(pressure, temperature);
}

void
LeadBismuthFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(pressure, temperature);
  dv_dp = 0;
  dv_dT = 1.293 / MathUtils::pow(11065 - 1.293 * temperature, 2);
}

Real
LeadBismuthFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  // see 2.55 in 2005 NEA Lead Handbook
  // 4.167e-6 is replaced by 1.25e-5/3 for accuracy
  return 164.8 * (temperature - _T_mo) - 1.97e-2 * (temperature * temperature - _T_mo * _T_mo) +
         (1.25e-5 / 3) * (temperature * temperature * temperature - _T_mo * _T_mo * _T_mo) +
         4.56e+5 * (1 / temperature - 1 / _T_mo);
}

void
LeadBismuthFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0;
  dh_dT = cp_from_p_T(pressure, temperature);
}

Real
LeadBismuthFluidProperties::h_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  return 164.8 * (temperature - _T_mo) - 1.97e-2 * (temperature * temperature - _T_mo * _T_mo) +
         (1.25e-5 / 3) * (temperature * temperature * temperature - _T_mo * _T_mo * _T_mo) +
         4.56e+5 * (1 / temperature - 1 / _T_mo);
}

void
LeadBismuthFluidProperties::h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);
  h = h_from_v_e(v, e);
  Real cp = cp_from_v_e(v, e);
  dh_dv = cp * dT_dv;
  dh_de = cp * dT_de;
}

Real
LeadBismuthFluidProperties::e_from_p_T(Real p, Real T) const
{
  // definition of h = e + p * v
  Real v = v_from_p_T(p, T);
  Real h = h_from_p_T(p, T);
  return h - p * v;
}

void
LeadBismuthFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  Real dh_dp, dv_dp, dh_dT, dv_dT, v, h;
  h_from_p_T(p, T, h, dh_dp, dh_dT);
  v_from_p_T(p, T, v, dv_dp, dv_dT);
  e = h_from_p_T(v, e);
  de_dp = dh_dp - v - dv_dp * p;
  de_dT = dh_dT - dv_dT * p;
}

Real
LeadBismuthFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_T(p, T_from_p_rho(p, rho));
}

void
LeadBismuthFluidProperties::e_from_p_rho(
    Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  Real T, dT_dp, dT_drho;
  T_from_p_rho(p, rho, T, dT_dp, dT_drho);
  Real de_dp_T, de_dT;
  e_from_p_T(pressure, T, e, de_dp_T, de_dT);
  de_dp = de_dp_T * 1 + de_dT * dT_dp;
  de_drho = de_dT * dT_drho;
}

Real
LeadBismuthFluidProperties::T_from_p_rho(Real /*pressure*/, Real rho) const
{
  return (rho - 11065) / -1.293;
}

void
LeadBismuthFluidProperties::T_from_p_rho(
    Real pressure, Real rho, Real & temperature, Real & dT_dp, Real & dT_drho) const
{
  temperature = T_from_p_rho(pressure, rho);
  dT_dp = 0;
  dT_drho = 1 / -1.293;
}

Real
LeadBismuthFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 164.8 - 3.94e-2 * temperature + 1.25e-5 * temperature * temperature -
         4.56e+5 / temperature / temperature;
}

void
LeadBismuthFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  dcp_dT =
      -3.94e-2 + 2 * 1.25e-5 * temperature + 2 * 4.56e+5 / temperature / temperature / temperature;
}

Real
LeadBismuthFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 4.94e-4 * std::exp(754.1 / temperature);
}

void
LeadBismuthFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(pressure, temperature);
  dmu_dp = 0;
  dmu_dT = -754.1 * 4.94e-4 * exp(754.1 / temperature) / temperature / temperature;
}

Real
LeadBismuthFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 3.284 + 1.617e-2 * temperature - 2.305e-6 * temperature * temperature;
}

void
LeadBismuthFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(pressure, temperature);
  dk_dp = 0;
  dk_dT = 1.617e-2 - 2 * 2.305e-6 * temperature;
}
