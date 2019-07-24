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
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

std::string
SinglePhaseFluidProperties::fluidName() const
{
  return std::string("");
}

Real
SinglePhaseFluidProperties::criticalPressure() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::criticalTemperature() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::criticalDensity() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::criticalInternalEnergy() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::triplePointPressure() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::triplePointTemperature() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::gamma_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e) / cv_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::gamma_from_v_e(
    Real v, Real e, Real & gamma, Real & dgamma_dv, Real & dgamma_de) const
{
  fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");

  dgamma_dv = 0.0;
  dgamma_de = 0.0;
  gamma = gamma_from_v_e(v, e);
}

Real
SinglePhaseFluidProperties::gamma_from_p_T(Real p, Real T) const
{
  return cp_from_p_T(p, T) / cv_from_p_T(p, T);
}

void
SinglePhaseFluidProperties::gamma_from_p_T(
    Real p, Real T, Real & gamma, Real & dgamma_dp, Real & dgamma_dT) const
{
  fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");

  dgamma_dp = 0.0;
  dgamma_dT = 0.0;
  gamma = gamma_from_p_T(p, T);
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

Real SinglePhaseFluidProperties::henryConstant(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::henryConstant(Real, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidProperties::vaporPressure(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::vaporPressure(Real T, Real & p, Real & dp_dT) const
{
  fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");

  dp_dT = 0.0;
  p = vaporPressure(T);
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
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
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
SinglePhaseFluidProperties::rho_mu_from_p_T(Real p, Real T, Real & rho, Real & mu) const
{
  rho = rho_from_p_T(p, T);
  mu = mu_from_p_T(p, T);
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
