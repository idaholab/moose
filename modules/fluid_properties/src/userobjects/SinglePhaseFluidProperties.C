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
  : FluidProperties(parameters)
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties() {}

Real
SinglePhaseFluidProperties::e_from_p_T(Real p, Real T) const
{
  const Real rho = rho_from_p_T(p, T);
  return e_from_p_rho(p, rho);
}

DualReal
SinglePhaseFluidProperties::e_from_p_T(const DualReal & p, const DualReal & T) const
{
  Real e = 0.0;
  Real pressure = p.value();
  Real temperature = T.value();
  Real de_dp = 0.0;
  Real de_dT = 0.0;
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);

  DualReal result = e;
  for (size_t i = 0; i < p.derivatives().size(); ++i)
    result.derivatives()[i] = p.derivatives()[i] * de_dp + T.derivatives()[i] * de_dT;

  return result;
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
  const Real rho = rho_from_p_T(p, T);
  return 1.0 / rho;
}

DualReal
SinglePhaseFluidProperties::v_from_p_T(const DualReal & p, const DualReal & T) const
{
  Real rawv = 0;
  Real rawp = p.value();
  Real rawT = T.value();
  Real dvdp = 0;
  Real dvdT = 0;
  v_from_p_T(rawp, rawT, rawv, dvdp, dvdT);

  DualReal result = rawv;
  for (size_t i = 0; i < p.derivatives().size(); i++)
    result.derivatives()[i] = p.derivatives()[i] * dvdp + T.derivatives()[i] * dvdT;
  return result;
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

void
SinglePhaseFluidProperties::beta_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}

DualReal
SinglePhaseFluidProperties::beta_from_p_T(const DualReal & p, const DualReal & T) const
{
  Real beta = 0.0;
  Real pressure = p.value();
  Real temperature = T.value();
  Real dbeta_dp = 0.0;
  Real dbeta_dT = 0.0;
  beta_from_p_T(pressure, temperature, beta, dbeta_dp, dbeta_dT);

  DualReal result = beta;
  for (size_t i = 0; i < p.derivatives().size(); ++i)
    result.derivatives()[i] = p.derivatives()[i] * dbeta_dp + T.derivatives()[i] * dbeta_dT;

  return result;
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
SinglePhaseFluidProperties::criticalInternalEnergy() const
{
  mooseError(name(), ": criticalInternalEnergy() is not implemented");
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
SinglePhaseFluidProperties::gamma_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e) / cv_from_v_e(v, e);
}

Real
SinglePhaseFluidProperties::gamma_from_p_T(Real p, Real T) const
{
  return cp_from_p_T(p, T) / cv_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::beta(Real p, Real T) const
{
  mooseDeprecated(name(), ": beta() is deprecated. Use beta_from_p_T() instead");

  return beta_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::henryConstantIAPWS(Real T, Real A, Real B, Real C) const
{
  const Real Tr = T / 647.096;
  const Real tau = 1.0 - Tr;

  const Real lnkh =
      A / Tr + B * std::pow(tau, 0.355) / Tr + C * std::pow(Tr, -0.41) * std::exp(tau);

  // The vapor pressure used in this formulation
  const std::vector<Real> a{
      -7.85951783, 1.84408259, -11.7866497, 22.6807411, -15.9618719, 1.80122502};
  const std::vector<Real> b{1.0, 1.5, 3.0, 3.5, 4.0, 7.5};
  Real sum = 0.0;

  for (std::size_t i = 0; i < a.size(); ++i)
    sum += a[i] * std::pow(tau, b[i]);

  return 22.064e6 * std::exp(sum / Tr) * std::exp(lnkh);
}

void
SinglePhaseFluidProperties::henryConstantIAPWS(
    Real T, Real & Kh, Real & dKh_dT, Real A, Real B, Real C) const
{
  const Real pc = 22.064e6;
  const Real Tc = 647.096;

  const Real Tr = T / Tc;
  const Real tau = 1.0 - Tr;

  const Real lnkh =
      A / Tr + B * std::pow(tau, 0.355) / Tr + C * std::pow(Tr, -0.41) * std::exp(tau);
  const Real dlnkh_dT =
      (-A / Tr / Tr - B * std::pow(tau, 0.355) / Tr / Tr - 0.355 * B * std::pow(tau, -0.645) / Tr -
       0.41 * C * std::pow(Tr, -1.41) * std::exp(tau) - C * std::pow(Tr, -0.41) * std::exp(tau)) /
      Tc;

  // The vapor pressure used in this formulation
  const std::vector<Real> a{
      -7.85951783, 1.84408259, -11.7866497, 22.6807411, -15.9618719, 1.80122502};
  const std::vector<Real> b{1.0, 1.5, 3.0, 3.5, 4.0, 7.5};
  Real sum = 0.0;
  Real dsum = 0.0;

  for (std::size_t i = 0; i < a.size(); ++i)
  {
    sum += a[i] * std::pow(tau, b[i]);
    dsum += a[i] * b[i] * std::pow(tau, b[i] - 1.0);
  }

  const Real p = pc * std::exp(sum / Tr);
  const Real dp_dT = -p / Tc / Tr * (sum / Tr + dsum);

  // Henry's constant and its derivative wrt temperature
  Kh = p * std::exp(lnkh);
  dKh_dT = (p * dlnkh_dT + dp_dT) * std::exp(lnkh);
}

void
SinglePhaseFluidProperties::henryConstantIAPWS_dT(
    Real T, Real & Kh, Real & dKh_dT, Real A, Real B, Real C) const
{
  mooseDeprecated(name(),
                  ":henryConstantIAPWS_dT() is deprecated. Use henryConstantIAPWS() instead");

  henryConstantIAPWS(T, Kh, dKh_dT, A, B, C);
}

void
SinglePhaseFluidProperties::mu_from_rho_T(Real, Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": mu_from_rho_T is not implemented.");
}

void
SinglePhaseFluidProperties::mu_drhoT_from_rho_T(
    Real rho, Real T, Real drho_dT, Real & mu, Real & dmu_drho, Real & dmu_dT) const
{
  mooseDeprecated(name(), ":mu_drhoT_from_rho_T() is deprecated. Use mu_from_rho_T() instead");

  mu_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);
}

Real SinglePhaseFluidProperties::henryConstant(Real) const
{
  mooseError(name(), ": henryConstant() is not implemented");
}

void
SinglePhaseFluidProperties::henryConstant(Real, Real &, Real &) const
{
  mooseError(name(), ": henryConstant() is not implemented");
}

void
SinglePhaseFluidProperties::henryConstant_dT(Real T, Real & Kh, Real & dKh_dT) const
{
  mooseDeprecated(name(), ": henryConstant_dT() is deprecated. Use henryConstant() instead");

  henryConstant(T, Kh, dKh_dT);
}

Real SinglePhaseFluidProperties::vaporPressure(Real) const
{
  mooseError(name(), ": vaporPressure() is not implemented");
}

void
SinglePhaseFluidProperties::vaporPressure(Real T, Real & p, Real & dp_dT) const
{
  fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");

  dp_dT = 0.0;
  p = vaporPressure(T);
}

void
SinglePhaseFluidProperties::vaporPressure_dT(Real T, Real & psat, Real & dpsat_dT) const
{
  mooseDeprecated(name(), ": vaporPressure_dT() is deprecated. Use vaporPressure() instead");

  vaporPressure(T, psat, dpsat_dT);
}

DualReal
SinglePhaseFluidProperties::vaporPressure(const DualReal & T) const
{
  Real p = 0.0;
  Real temperature = T.value();
  Real dpdT = 0.0;

  vaporPressure(temperature, p, dpdT);

  DualReal result = p;
  for (std::size_t i = 0; i < T.derivatives().size(); ++i)
    result.derivatives()[i] = T.derivatives()[i] * dpdT;

  return result;
}

Real SinglePhaseFluidProperties::vaporTemperature(Real) const
{
  mooseError(name(), ": vaporTemperature() is not implemented");
}

void
SinglePhaseFluidProperties::vaporTemperature(Real p, Real & T, Real & dT_dp) const
{
  fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");

  dT_dp = 0.0;
  T = vaporTemperature(p);
}

DualReal
SinglePhaseFluidProperties::vaporTemperature(const DualReal & p) const
{
  Real T = 0.0;
  Real pressure = p.value();
  Real dTdp = 0.0;

  vaporTemperature(pressure, T, dTdp);

  DualReal result = T;
  for (std::size_t i = 0; i < p.derivatives().size(); ++i)
    result.derivatives()[i] = p.derivatives()[i] * dTdp;

  return result;
}

void
SinglePhaseFluidProperties::rho_dpT(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  mooseDeprecated(name(), ": rho_dpT() is deprecated. Use rho_from_p_T() instead");

  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
}

Real
SinglePhaseFluidProperties::e(Real p, Real T) const
{
  mooseDeprecated(name(), ": e() is deprecated. Use e_from_p_T() instead");

  return e_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::e_dpT(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  mooseDeprecated(name(), ": e_dpT() is deprecated. Use e_from_p_T() instead");

  e_from_p_T(p, T, e, de_dp, de_dT);
}

void
SinglePhaseFluidProperties::rho_e_from_p_T(Real p,
                                           Real T,
                                           Real & rho,
                                           Real & drho_dp,
                                           Real & drho_dT,
                                           Real & e,
                                           Real & de_dp,
                                           Real & de_dT) const
{
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  e_from_p_T(p, T, e, de_dp, de_dT);
}

void
SinglePhaseFluidProperties::rho_e_dpT(Real p,
                                      Real T,
                                      Real & rho,
                                      Real & drho_dp,
                                      Real & drho_dT,
                                      Real & e,
                                      Real & de_dp,
                                      Real & de_dT) const
{
  mooseDeprecated(name(), ": rho_e_dpT() is deprecated. Use rho_e_from_p_T() instead");

  rho_e_from_p_T(p, T, rho, drho_dp, drho_dT, e, de_dp, de_dT);
}

Real
SinglePhaseFluidProperties::c(Real p, Real T) const
{
  mooseDeprecated(name(), ": c() is deprecated. Use c_from_p_T() instead");

  return c_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::mu(Real p, Real T) const
{
  mooseDeprecated(name(), ": mu() is deprecated. Use mu_from_p_T() instead");

  return mu_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::mu_dpT(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mooseDeprecated(name(), ": mu_dpT() is deprecated. Use mu_from_p_T() instead");

  mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
}

void
SinglePhaseFluidProperties::rho_mu(Real p, Real T, Real & rho, Real & mu) const
{
  mooseDeprecated(name(), ": rho_mu() is deprecated. Use rho_mu_from_p_T() instead");

  rho_mu_from_p_T(p, T, rho, mu);
}

void
SinglePhaseFluidProperties::rho_mu_from_p_T(Real p, Real T, Real & rho, Real & mu) const
{
  rho = rho_from_p_T(p, T);
  mu = mu_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::rho_mu_dpT(Real p,
                                       Real T,
                                       Real & rho,
                                       Real & drho_dp,
                                       Real & drho_dT,
                                       Real & mu,
                                       Real & dmu_dp,
                                       Real & dmu_dT) const
{
  mooseDeprecated(name(), ": rho_mu_dpT() is deprecated. Use rho_mu_from_p_T() instead");

  rho_mu_from_p_T(p, T, rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT);
}

void
SinglePhaseFluidProperties::rho_mu_from_p_T(Real p,
                                            Real T,
                                            Real & rho,
                                            Real & drho_dp,
                                            Real & drho_dT,
                                            Real & mu,
                                            Real & dmu_dp,
                                            Real & dmu_dT) const
{
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
}

Real
SinglePhaseFluidProperties::k(Real p, Real T) const
{
  mooseDeprecated(name(), ": k() is deprecated. Use k_from_p_T() instead");

  return k_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::k_dpT(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  mooseDeprecated(name(), ": k_dpT() is deprecated. Use k_from_p_T() instead");

  k_from_p_T(p, T, k, dk_dp, dk_dT);
}

Real
SinglePhaseFluidProperties::s(Real p, Real T) const
{
  mooseDeprecated(name(), ": s() is deprecated. Use s_from_p_T() instead");

  return s_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::h(Real p, Real T) const
{
  mooseDeprecated(name(), ": h() is deprecated. Use h_from_p_T() instead");

  return h_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::h_dpT(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  mooseDeprecated(name(), ": h_dpT() is deprecated. Use h_from_p_T() instead");

  h_from_p_T(p, T, h, dh_dp, dh_dT);
}

Real
SinglePhaseFluidProperties::rho(Real p, Real T) const
{
  mooseDeprecated(name(), ": rho() is deprecated. Use rho_from_p_T() instead");

  return rho_from_p_T(p, T);
}

Real SinglePhaseFluidProperties::e_spndl_from_v(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::v_e_spndl_from_T(Real, Real &, Real &) const
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

void
SinglePhaseFluidProperties::T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const
{
  fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");

  dT_dp = 0.0;
  dT_dh = 0.0;
  T = T_from_p_h(p, h);
}

DualReal
SinglePhaseFluidProperties::T_from_p_h(const DualReal & p, const DualReal & h) const
{
  Real T = 0.0;
  Real pressure = p.value();
  Real enthalpy = h.value();
  Real dT_dp = 0.0;
  Real dT_dh = 0.0;
  T_from_p_h(pressure, enthalpy, T, dT_dp, dT_dh);

  DualReal result = T;
  for (size_t i = 0; i < p.derivatives().size(); ++i)
    result.derivatives()[i] = p.derivatives()[i] * dT_dp + h.derivatives()[i] * dT_dh;

  return result;
}
