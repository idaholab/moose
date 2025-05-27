//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeadLithiumFluidProperties.h"

registerMooseObject("FluidPropertiesApp", LeadLithiumFluidProperties);

InputParameters
LeadLithiumFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for Lead Lithium eutectic (83% Pb, 17% Li)");
  return params;
}

LeadLithiumFluidProperties::LeadLithiumFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

std::string
LeadLithiumFluidProperties::fluidName() const
{
  return "LeadLithium";
}

Real
LeadLithiumFluidProperties::molarMass() const
{
  // Approximate molar mass for 83% Pb (207.2 g/mol) and 17% Li (6.94 g/mol) by atomic fraction
  return 1.73e-1; // in kg/mol
}

Real
LeadLithiumFluidProperties::rho_from_p_T(Real /*p*/, Real T) const
{
  if (T < _T_mo || T > 880)
    flagInvalidSolution("Temperature out of bounds for the PbLi density computation");
  return 10520.35 - 1.19051 * T;
}

void
LeadLithiumFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = 0;
  drho_dT = -1.19051;
}

void
LeadLithiumFluidProperties::rho_from_p_T(
    const ADReal & p, const ADReal & T, ADReal & rho, ADReal & drho_dp, ADReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(p, T);
  drho_dp = 0;
  drho_dT = -1.19051;
}

Real
LeadLithiumFluidProperties::v_from_p_T(Real p, Real T) const
{
  return 1.0 / rho_from_p_T(p, T);
}

void
LeadLithiumFluidProperties::v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(p, T);
  dv_dp = 0;
  dv_dT = 1.19051 / Utility::pow<2>(10520.35 - 1.19051 * T);
}

Real
LeadLithiumFluidProperties::T_from_v_e(Real v, Real /*e*/) const
{
  Real T = (10520.35 - 1.0 / v) / 1.19051;
  // This is computed from rho(T), thus shares the same validity range
  if (T < _T_mo || T > 880)
    flagInvalidSolution("Specific volume out of bounds for the PbLi temperature computation");
  return T;
}

void
LeadLithiumFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_de = 0;
  dT_dv = 1.0 / (1.19051 * v * v);
}

Real
LeadLithiumFluidProperties::bulk_modulus_from_p_T(Real /*p*/, Real T) const
{
  return Utility::pow<2>(c_from_p_T(0, T)) * rho_from_p_T(0, T);
}

Real
LeadLithiumFluidProperties::c_from_p_T(Real /*p*/, Real T) const
{
  return 1876 - 0.306 * T;
}

Real
LeadLithiumFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 1876 - 0.306 * T;
}

ADReal
LeadLithiumFluidProperties::c_from_v_e(const ADReal & v, const ADReal & e) const
{
  ADReal T = SinglePhaseFluidProperties::T_from_v_e(v, e);
  return 1876 - 0.306 * T;
}

Real
LeadLithiumFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real h = h_from_v_e(v, e);
  return (h - e) / v;
}

void
LeadLithiumFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  Real h, dh_dv, dh_de;
  h_from_v_e(v, e, h, dh_dv, dh_de);
  dp_dv = (v * dh_dv - h + e) / (v * v);
  dp_de = (dh_de - 1) / v;
}

Real
LeadLithiumFluidProperties::cp_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  if (T < _T_mo || T > 800)
    flagInvalidSolution("Temperature out of bounds for the PbLi specific heat computation");
  return 195.0 - 9.116e-3 * T;
}

void
LeadLithiumFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  cp = cp_from_v_e(v, e);
  const Real dcp_dT = -9.116e-3;
  dcp_dv = dcp_dT * dT_dv;
  dcp_de = dcp_dT * dT_de;
}

Real
LeadLithiumFluidProperties::cv_from_p_T(Real p, Real T) const
{
  Real rho, drho_dT, drho_dp;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real alpha = -drho_dT / rho;
  Real bulk_modulus = bulk_modulus_from_p_T(p, T);
  Real cp = cp_from_p_T(p, T);
  return cp / (1.0 + alpha * alpha * bulk_modulus * T / (rho * cp));
}

void
LeadLithiumFluidProperties::cv_from_p_T(
    Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(p, T);
  // A full analytical derivative is complex; here we assume minimal pressure dependence.
  dcv_dp = 0;
  const Real dT = 1e-6;
  Real cv_plus = cv_from_p_T(p, T + dT);
  dcv_dT = (cv_plus - cv) / dT;
}

Real
LeadLithiumFluidProperties::cv_from_v_e(Real v, Real e) const
{
  Real p = p_from_v_e(v, e);
  Real T = T_from_v_e(v, e);
  return cv_from_p_T(p, T);
}

void
LeadLithiumFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  Real p, dp_dv, dp_de;
  p_from_v_e(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  Real dcv_dp, dcv_dT;
  cv_from_p_T(p, T, cv, dcv_dp, dcv_dT);
  dcv_dv = dcv_dp * dp_dv + dcv_dT * dT_dv;
  dcv_de = dcv_dp * dp_de + dcv_dT * dT_de;
}

Real
LeadLithiumFluidProperties::mu_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  if (T < _T_mo || T > 625)
    flagInvalidSolution("Temperature out of bounds for the PbLi viscosity computation");
  return 1.87e-4 * std::exp(11640.0 / (FluidProperties::_R * T));
}

void
LeadLithiumFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  mu = mu_from_v_e(v, e);
  Real factor = -11640.0 / (FluidProperties::_R * T * T);
  dmu_dv = factor * mu * dT_dv;
  dmu_de = factor * mu * dT_de;
}

Real
LeadLithiumFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  if (T < _T_mo || T > 873)
    flagInvalidSolution("Temperature out of bounds for the PbLi dynamic viscosity computation");
  return 14.51 + 1.9631e-2 * T;
}

void
LeadLithiumFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  k = k_from_v_e(v, e);
  dk_dv = 0.019631 * dT_dv;
  dk_de = 0.019631 * dT_de;
}

Real
LeadLithiumFluidProperties::h_from_p_T(Real /*p*/, Real T) const
{
  if (T < _T_mo || T > 800)
    flagInvalidSolution("Temperature out of bounds for the PbLi enthalpy computation");
  return 195.0 * (T - _T_mo) - 0.5 * 9.116e-3 * (T * T - _T_mo * _T_mo);
}

void
LeadLithiumFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  dh_dp = 0;
  dh_dT = cp_from_p_T(p, T);
}

Real
LeadLithiumFluidProperties::h_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return h_from_p_T(0, T);
}

void
LeadLithiumFluidProperties::h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  h = h_from_v_e(v, e);
  Real cp = cp_from_v_e(v, e);
  dh_dv = cp * dT_dv;
  dh_de = cp * dT_de;
}

Real
LeadLithiumFluidProperties::e_from_p_T(Real p, Real T) const
{
  Real v = v_from_p_T(p, T);
  Real h = h_from_p_T(p, T);
  return h - p * v;
}

void
LeadLithiumFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  Real dh_dp, dv_dp, dh_dT, dv_dT, v, h;
  h_from_p_T(p, T, h, dh_dp, dh_dT);
  v_from_p_T(p, T, v, dv_dp, dv_dT);
  e = e_from_p_T(p, T);
  de_dp = dh_dp - v - dv_dp * p;
  de_dT = dh_dT - dv_dT * p;
}

Real
LeadLithiumFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_T(p, T_from_p_rho(p, rho));
}

void
LeadLithiumFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  Real T, dT_dp, dT_drho;
  T_from_p_rho(p, rho, T, dT_dp, dT_drho);
  Real de_dp_T, de_dT;
  e_from_p_T(p, T, e, de_dp_T, de_dT);
  de_dp = de_dp_T + de_dT * dT_dp;
  de_drho = de_dT * dT_drho;
}

Real
LeadLithiumFluidProperties::T_from_p_rho(Real /*p*/, Real rho) const
{
  Real T = (10520.35 - rho) / 1.19051;
  if (T < _T_mo || T > 880)
    flagInvalidSolution("Temperature out of bounds for the PbLi density computation");
  return T;
}

void
LeadLithiumFluidProperties::T_from_p_rho(
    Real p, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const
{
  T = T_from_p_rho(p, rho);
  dT_dp = 0;
  dT_drho = -1.0 / 1.19051;
}

Real
LeadLithiumFluidProperties::T_from_p_h(Real /*p*/, Real h) const
{
  // h = 195.0 * (T - _T_mo) - 0.5 * 9.116e-3 * (T * T - _T_mo * _T_mo);
  //
  const auto a = -0.5 * 9.116e-3;
  const auto b = 195.;
  const auto c = 0.5 * 9.116e-3 * _T_mo * _T_mo - h - 195. * _T_mo;
  const auto T = (-b + std::sqrt(b * b - 4 * a * c)) / (2 * a);
  return T;
}

void
LeadLithiumFluidProperties::T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const
{
  T = T_from_p_h(p, h);
  dT_dp = 0;
  Real h1, dh_dp, dh_dT;
  h_from_p_T(p, T, h1, dh_dp, dh_dT);
  dT_dh = 1.0 / dh_dT;
}

Real
LeadLithiumFluidProperties::cp_from_p_T(Real /*p*/, Real T) const
{
  if (T < _T_mo || T > 800)
    flagInvalidSolution("Temperature out of bounds for the PbLi specific heat computation");
  return 195.0 - 9.116e-3 * T;
}

void
LeadLithiumFluidProperties::cp_from_p_T(
    Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(p, T);
  dcp_dp = 0;
  dcp_dT = -9.116e-3;
}

Real
LeadLithiumFluidProperties::mu_from_p_T(Real /*p*/, Real T) const
{
  if (T < _T_mo || T > 625)
    flagInvalidSolution("Temperature out of bounds for the PbLi viscosity computation");
  return 1.87e-4 * std::exp(11640.0 / (FluidProperties::_R * T));
}

void
LeadLithiumFluidProperties::mu_from_p_T(
    Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(p, T);
  dmu_dp = 0;
  dmu_dT = -11640.0 / (FluidProperties::_R * T * T) * mu;
}

Real
LeadLithiumFluidProperties::k_from_p_T(Real /*p*/, Real T) const
{
  if (T < _T_mo || T > 873)
    flagInvalidSolution("Temperature out of bounds for the PbLi dynamic viscosity computation");
  return 14.51 + 1.9631e-2 * T;
}

void
LeadLithiumFluidProperties::k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(p, T);
  dk_dp = 0;
  dk_dT = 0.019631;
}
