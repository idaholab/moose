//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<SinglePhaseFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidProperties::SinglePhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters), _R(8.3144598), _T_c2k(273.15)
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties() {}

Real SinglePhaseFluidProperties::p_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::p_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::T_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::T_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::c_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::c_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::cp_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::cv_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::mu_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::k_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::s_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::s_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::s_from_h_p(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::s_from_h_p(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::rho_from_p_s(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::rho_from_p_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::e_from_v_h(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::e_from_v_h(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::e_from_p_T(Real p, Real T) const
{
  Real rho = rho_from_p_T(p, T);
  return e_from_p_rho(p, rho);
}

void
SinglePhaseFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  // From rho(p,T), compute: drho(p,T)/dp, drho(p,T)/dT
  Real rho = 0., drho_dp = 0., drho_dT = 0.;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  // From e(p, rho), compute: de(p,rho)/dp, de(p,rho)/drho
  Real depr_dp = 0., depr_drho = 0.;
  e_from_p_rho(p, rho, e, depr_dp, depr_drho);

  // Using partial derivative rules, we have:
  // de(p,T)/dp = de(p,rho)/dp * dp/dp + de(p,rho)/drho * drho(p,T)/dp, (dp/dp == 1)
  // de(p,T)/dT = de(p,rho)/dp * dp/dT + de(p,rho)/drho * drho(p,T)/dT, (dp/dT == 0)
  de_dp = depr_dp + depr_drho * drho_dp;
  de_dT = depr_drho * drho_dT;
}

Real
SinglePhaseFluidProperties::v_from_p_T(Real p, Real T) const
{
  Real rho = rho_from_p_T(p, T);
  return 1.0 / rho;
}

void
SinglePhaseFluidProperties::v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  v = 1.0 / rho;
  const Real dv_drho = -1.0 / (rho * rho);

  dv_dp = dv_drho * drho_dp;
  dv_dT = dv_drho * drho_dT;
}

Real
SinglePhaseFluidProperties::beta_from_p_T(Real p, Real T) const
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

Real SinglePhaseFluidProperties::s_from_p_T(Real, Real) const
{
  mooseError(name(), ": s_from_p_T is not implemented");
}

void
SinglePhaseFluidProperties::s_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": s_from_p_T is not implemented");
}

Real
SinglePhaseFluidProperties::molarMass() const
{
  mooseError(name(), ": molarMass is not implemented");
}

std::string
SinglePhaseFluidProperties::fluidName() const
{
  return std::string("");
}

Real
SinglePhaseFluidProperties::criticalPressure() const
{
  mooseError(name(), ": criticalPressure() is not implemented");
}

Real
SinglePhaseFluidProperties::criticalTemperature() const
{
  mooseError(name(), ": criticalTemperature() is not implemented");
}

Real
SinglePhaseFluidProperties::criticalDensity() const
{
  mooseError(name(), ": criticalDensity() is not implemented");
}

Real
SinglePhaseFluidProperties::triplePointPressure() const
{
  mooseError(name(), ": triplePointPressure() is not implemented");
}

Real
SinglePhaseFluidProperties::triplePointTemperature() const
{
  mooseError(name(), ": triplePointTemperature() is not implemented");
}

Real
SinglePhaseFluidProperties::gamma_from_p_T(Real pressure, Real temperature) const
{
  return cp_from_p_T(pressure, temperature) / cv_from_p_T(pressure, temperature);
}

Real
SinglePhaseFluidProperties::beta(Real pressure, Real temperature) const
{
  return beta_from_p_T(pressure, temperature);
}

Real
SinglePhaseFluidProperties::henryConstantIAPWS(Real temperature, Real A, Real B, Real C) const
{
  Real Tr = temperature / 647.096;
  Real tau = 1.0 - Tr;

  Real lnkh = A / Tr + B * std::pow(tau, 0.355) / Tr + C * std::pow(Tr, -0.41) * std::exp(tau);

  // The vapor pressure used in this formulation
  std::vector<Real> a{-7.85951783, 1.84408259, -11.7866497, 22.6807411, -15.9618719, 1.80122502};
  std::vector<Real> b{1.0, 1.5, 3.0, 3.5, 4.0, 7.5};
  Real sum = 0.0;

  for (std::size_t i = 0; i < a.size(); ++i)
    sum += a[i] * std::pow(tau, b[i]);

  return 22.064e6 * std::exp(sum / Tr) * std::exp(lnkh);
}

void
SinglePhaseFluidProperties::henryConstantIAPWS_dT(
    Real temperature, Real & Kh, Real & dKh_dT, Real A, Real B, Real C) const
{
  Real pc = 22.064e6;
  Real Tc = 647.096;

  Real Tr = temperature / Tc;
  Real tau = 1.0 - Tr;

  Real lnkh = A / Tr + B * std::pow(tau, 0.355) / Tr + C * std::pow(Tr, -0.41) * std::exp(tau);
  Real dlnkh_dT =
      (-A / Tr / Tr - B * std::pow(tau, 0.355) / Tr / Tr - 0.355 * B * std::pow(tau, -0.645) / Tr -
       0.41 * C * std::pow(Tr, -1.41) * std::exp(tau) - C * std::pow(Tr, -0.41) * std::exp(tau)) /
      Tc;

  // The vapor pressure used in this formulation
  std::vector<Real> a{-7.85951783, 1.84408259, -11.7866497, 22.6807411, -15.9618719, 1.80122502};
  std::vector<Real> b{1.0, 1.5, 3.0, 3.5, 4.0, 7.5};
  Real sum = 0.0;
  Real dsum = 0.0;

  for (std::size_t i = 0; i < a.size(); ++i)
  {
    sum += a[i] * std::pow(tau, b[i]);
    dsum += a[i] * b[i] * std::pow(tau, b[i] - 1.0);
  }

  Real p = pc * std::exp(sum / Tr);
  Real dp_dT = -p / Tc / Tr * (sum / Tr + dsum);

  // Henry's constant and its derivative wrt temperature
  Kh = p * std::exp(lnkh);
  dKh_dT = (p * dlnkh_dT + dp_dT) * std::exp(lnkh);
}

Real SinglePhaseFluidProperties::mu_from_rho_T(Real, Real) const
{
  mooseError(name(), ": mu_from_rho_T is not implemented.");
}

void
SinglePhaseFluidProperties::mu_drhoT_from_rho_T(Real, Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": mu_drhoT_from_rho_T is not implemented.");
}

Real SinglePhaseFluidProperties::k_from_rho_T(Real /*density*/, Real /*temperature*/) const
{
  mooseError(name(), ": k_from_rho_T is not implemented.");
}

Real SinglePhaseFluidProperties::henryConstant(Real /*temperature*/) const
{
  mooseError(name(), ": henryConstant() is not implemented");
}

void
SinglePhaseFluidProperties::henryConstant_dT(Real /*temperature*/,
                                             Real & /*Kh*/,
                                             Real & /*dKh_dT*/) const
{
  mooseError(name(), ": henryConstant_dT() is not implemented");
}

Real SinglePhaseFluidProperties::vaporPressure(Real /*temperature*/) const
{
  mooseError(name(), ": vaporPressure() is not implemented");
}

void
SinglePhaseFluidProperties::vaporPressure_dT(Real /*temperature*/,
                                             Real & /*psat*/,
                                             Real & /*dpsat_dT*/) const
{
  mooseError(name(), ": vaporPressure_dT() is not implemented");
}

void
SinglePhaseFluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
}

Real
SinglePhaseFluidProperties::e(Real p, Real T) const
{
  return e_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::e_dpT(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);
}

void
SinglePhaseFluidProperties::rho_e_dpT(
    Real, Real, Real &, Real &, Real &, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::c_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::c(Real pressure, Real temperature) const
{
  return c_from_p_T(pressure, temperature);
}

Real SinglePhaseFluidProperties::cp_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::cv_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::mu(Real pressure, Real temperature) const
{
  return mu_from_p_T(pressure, temperature);
}

Real SinglePhaseFluidProperties::mu_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::mu_dpT(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
}

void
SinglePhaseFluidProperties::mu_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::rho_mu(Real, Real, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::rho_mu_dpT(
    Real, Real, Real &, Real &, Real &, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::k_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::k(Real p, Real T) const
{
  return k_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::k_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::k_dpT(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k_from_p_T(pressure, temperature, k, dk_dp, dk_dT);
}

Real
SinglePhaseFluidProperties::s(Real p, Real T) const
{
  return s_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::h(Real p, Real T) const
{
  return h_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::h_dpT(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h_from_p_T(p, T, h, dh_dp, dh_dT);
}

Real SinglePhaseFluidProperties::rho_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::rho(Real p, Real T) const
{
  return rho_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::rho_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::e_from_p_rho(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::e_from_p_rho(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::p_from_T_v(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::h_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::h_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::p_from_h_s(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::p_from_h_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::g_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::T_from_p_h(Real p, Real h) const
{
  const Real s = s_from_h_p(h, p);
  const Real rho = rho_from_p_s(p, s);
  const Real v = 1. / rho;
  const Real e = e_from_v_h(v, h);
  return T_from_v_e(v, e);
}
