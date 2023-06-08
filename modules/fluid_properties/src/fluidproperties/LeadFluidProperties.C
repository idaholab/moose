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
  params.addParam<Real>("T_mo", 600.6, "Melting Point (K)");
  params.addClassDescription("Fluid properties for Lead");

  return params;
}

LeadFluidProperties::LeadFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _T_mo(getParam<Real>("T_mo"))
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
  return 2.072e-1;
}

Real
LeadFluidProperties::bulk_modulus_from_p_T(Real /*p*/, Real T) const
{
  // Isentropic bulk modulus, eq 2.42 from Handbook
  return (43.50 - 1.552e-2 * T + 1.622e-6 * T * T) * 1e9;
}

Real
LeadFluidProperties::c_from_v_e(Real v, Real e) const
{
  auto T = T_from_v_e(v, e);
  return 1953 - 0.246 * T;
}

DualReal
LeadFluidProperties::c_from_v_e(const DualReal & v, const DualReal & e) const
{
  DualReal T = SinglePhaseFluidProperties::T_from_v_e(v, e);
  return 1953 - 0.246 * T;
}

Real
LeadFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real h = h_from_v_e(v, e);
  return (h - e) / v;
}

void
LeadFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  Real h, dh_dv, dh_de;
  h_from_v_e(v, e, h, dh_dv, dh_de);
  p = p_from_v_e(v, e);
  dp_dv = (v * dh_dv - h + e) / v / v;
  dp_de = (dh_de - 1) / v;
}

Real
LeadFluidProperties::mu_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 4.55e-4 * std::exp(1069 / T);
}

void
LeadFluidProperties::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  mu = mu_from_v_e(v, e);
  dmu_dv = dT_dv * -1069 * 4.55e-4 * exp(1069 / T) / T / T;
  dmu_de = dT_de * -1069 * 4.55e-4 * exp(1069 / T) / T / T;
}

Real
LeadFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 0.011 * T + 9.2;
}

void
LeadFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  k = k_from_v_e(v, e);
  dk_dv = 0.011 * dT_dv;
  dk_de = 0.011 * dT_de;
}

Real
LeadFluidProperties::rho_from_p_T(Real /*p*/, Real T) const
{
  return 11441 - 1.2795 * T;
}

void
LeadFluidProperties::rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = 0;
  drho_dT = -1.2795;
}

void
LeadFluidProperties::rho_from_p_T(const DualReal & p,
                                  const DualReal & T,
                                  DualReal & rho,
                                  DualReal & drho_dp,
                                  DualReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(p, T);
  drho_dp = 0;
  drho_dT = -1.2795;
}

Real
LeadFluidProperties::v_from_p_T(Real p, Real T) const
{
  return 1.0 / rho_from_p_T(p, T);
}

void
LeadFluidProperties::v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(p, T);
  dv_dp = 0;
  dv_dT = 1.2795 / MathUtils::pow(11441 - 1.2795 * T, 2);
}

Real
LeadFluidProperties::h_from_p_T(Real /*p*/, Real T) const
{
  // see 2.53 in 2005 NEA Lead Handbook
  // 5.1467e-6 is replaced by 1.544e-5/3 for accuracy
  return 176.2 * (T - _T_mo) - 2.4615e-2 * (T * T - _T_mo * _T_mo) +
         (1.544e-5 / 3) * (T * T * T - _T_mo * _T_mo * _T_mo) + 1.524e+6 * (1 / T - 1 / _T_mo);
}

void
LeadFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  Real cp = cp_from_p_T(p, T);
  dh_dp = 0;
  dh_dT = cp;
}

Real
LeadFluidProperties::h_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 176.2 * (T - _T_mo) - 2.4615e-2 * (T * T - _T_mo * _T_mo) +
         (1.544e-5 / 3) * (T * T * T - _T_mo * _T_mo * _T_mo) + 1.524e+6 * (1 / T - 1 / _T_mo);
}

void
LeadFluidProperties::h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  h = h_from_v_e(v, e);
  Real cp = cp_from_v_e(v, e);
  dh_dv = cp * dT_dv;
  dh_de = cp * dT_de;
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
  e = e_from_p_T(p, T);
  de_dp = dh_dp - v - dv_dp * p;
  de_dT = dh_dT - dv_dT * p;
}

Real
LeadFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_T(p, T_from_p_rho(p, rho));
}

void
LeadFluidProperties::e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  Real T, dT_dp, dT_drho;
  T_from_p_rho(p, rho, T, dT_dp, dT_drho);
  Real de_dp_T, de_dT;
  e_from_p_T(p, T, e, de_dp_T, de_dT);
  de_dp = de_dp_T * 1 + de_dT * dT_dp;
  de_drho = de_dT * dT_drho;
}

Real
LeadFluidProperties::T_from_p_rho(Real /*p*/, Real rho) const
{
  return (rho - 11441) / -1.2795;
}

void
LeadFluidProperties::T_from_p_rho(Real p, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const
{
  T = T_from_p_rho(p, rho);
  dT_dp = 0;
  dT_drho = 1 / -1.2795;
}

Real
LeadFluidProperties::T_from_v_e(Real v, Real /*e*/) const
{
  return (1 / v - 11441) / -1.2795;
}

void
LeadFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_de = 0;
  dT_dv = 1. / v / v / 1.2795;
}

Real
LeadFluidProperties::cp_from_p_T(Real /*p*/, Real T) const
{
  return 176.2 - 4.923e-2 * T + 1.544e-5 * T * T - 1.524e+6 / T / T;
}

void
LeadFluidProperties::cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(p, T);
  dcp_dp = 0;
  dcp_dT = -4.923e-2 + 2 * 1.544e-5 * T + 2 * 1.524e6 / T / T / T;
}

Real
LeadFluidProperties::cp_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 176.2 - 4.923e-2 * T + 1.544e-5 * T * T - 1.524e6 / T / T;
}

void
LeadFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  cp = cp_from_v_e(v, e);
  dcp_dv = -4.923e-2 * dT_dv + 2 * dT_dv * 1.544e-5 * T + 2 * dT_dv * 1.524e+6 / T / T / T;

  dcp_de = -4.923e-2 * dT_de + 2 * dT_de * 1.544e-5 * T + 2 * dT_de * 1.524e+6 / T / T / T;
}

Real
LeadFluidProperties::cv_from_v_e(Real v, Real e) const
{
  Real p = p_from_v_e(v, e);
  Real T = T_from_v_e(v, e);
  return cv_from_p_T(p, T);
}

void
LeadFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
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
LeadFluidProperties::cv_from_p_T(Real p, Real T) const
{
  Real rho, drho_dT, drho_dp;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real alpha = -drho_dT / rho;
  Real bulk_modulus = bulk_modulus_from_p_T(p, T);
  Real cp = cp_from_p_T(p, T);
  return cp / (1 + alpha * alpha * bulk_modulus * T / rho / cp);
}

void
LeadFluidProperties::cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  Real cp, dcp_dp, dcp_dT;
  cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
  cv = cv_from_p_T(p, T);

  Real rho, drho_dT, drho_dp;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real alpha = -drho_dT / rho;
  Real alpha_2 = alpha * alpha, alpha_3 = alpha * alpha * alpha;
  Real dalpha_dT = drho_dT * drho_dT / rho / rho;
  Real bulk = bulk_modulus_from_p_T(p, T);
  Real dbulk_dT = (-1.552e-2 + 2 * 1.622e-6 * T) * 1e9;
  Real denominator = (1 + alpha * alpha * bulk * T / rho / cp);
  // no pressure dependence in alpha, T, bulk modulus, cp or rho
  dcv_dp = 0;
  dcv_dT = dcp_dT / denominator -
           cp / denominator / denominator *
               (2 * alpha * dalpha_dT * bulk * T / rho / cp + alpha_2 * dbulk_dT * T / rho / cp +
                alpha_2 * bulk / rho / cp + alpha_3 * bulk * T / rho / cp -
                dcp_dT * alpha_2 * bulk * T / rho / cp / cp);
}

Real
LeadFluidProperties::mu_from_p_T(Real /*p*/, Real T) const
{
  return 4.55e-4 * std::exp(1069 / T);
}

void
LeadFluidProperties::mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(p, T);
  dmu_dp = 0;
  dmu_dT = -1069 * 4.55e-4 * std::exp(1069 / T) / T / T;
}

Real
LeadFluidProperties::k_from_p_T(Real /*p*/, Real T) const
{
  return 0.011 * T + 9.2;
}

void
LeadFluidProperties::k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(p, T);
  dk_dp = 0;
  dk_dT = 0.011;
}
