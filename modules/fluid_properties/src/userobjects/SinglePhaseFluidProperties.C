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
  params.addCustomTypeParam<std::string>(
      "fp_type", "single-phase-fp", "FPType", "Type of the fluid property object");
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

Real SinglePhaseFluidProperties::vaporPressure(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

std::vector<Real>
SinglePhaseFluidProperties::henryCoefficients() const
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
  result.derivatives() = T.derivatives() * dpdT;

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
  result.derivatives() = p.derivatives() * dTdp;

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

void
SinglePhaseFluidProperties::rho_mu_from_p_T(const DualReal & p,
                                            const DualReal & T,
                                            DualReal & rho,
                                            DualReal & mu) const
{
  rho = rho_from_p_T(p, T);
  mu = mu_from_p_T(p, T);
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
  Real s, ds_dh, ds_dp;
  s_from_h_p(h, p, s, ds_dh, ds_dp);

  Real rho, drho_dp_partial, drho_ds;
  rho_from_p_s(p, s, rho, drho_dp_partial, drho_ds);
  const Real drho_dp = drho_dp_partial + drho_ds * ds_dp;
  const Real drho_dh = drho_ds * ds_dh;

  const Real v = 1.0 / rho;
  const Real dv_drho = -1.0 / (rho * rho);
  const Real dv_dp = dv_drho * drho_dp;
  const Real dv_dh = dv_drho * drho_dh;

  Real e, de_dv, de_dh_partial;
  e_from_v_h(v, h, e, de_dv, de_dh_partial);
  const Real de_dp = de_dv * dv_dp;
  const Real de_dh = de_dh_partial + de_dv * dv_dh;

  Real dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  dT_dp = dT_dv * dv_dp + dT_de * de_dp;
  dT_dh = dT_dv * dv_dh + dT_de * de_dh;
}
