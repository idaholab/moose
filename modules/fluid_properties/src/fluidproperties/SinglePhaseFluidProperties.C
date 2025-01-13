//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidProperties.h"

InputParameters
SinglePhaseFluidProperties::validParams()
{
  InputParameters params = FluidProperties::validParams();
  params.set<std::string>("fp_type") = "single-phase-fp";

  // Variable set conversion parameters
  params.addRangeCheckedParam<Real>(
      "tolerance", 1e-8, "tolerance > 0", "Tolerance for 2D Newton variable set conversion");
  params.addRangeCheckedParam<Real>(
      "T_initial_guess",
      400,
      "T_initial_guess > 0",
      "Temperature initial guess for Newton Method variable set conversion");
  params.addRangeCheckedParam<Real>(
      "p_initial_guess",
      2e5,
      "p_initial_guess > 0",
      "Pressure initial guess for Newton Method variable set conversion");
  params.addParam<unsigned int>(
      "max_newton_its", 100, "Maximum number of Newton iterations for variable set conversions");
  params.addParamNamesToGroup("tolerance T_initial_guess p_initial_guess",
                              "Variable set conversions Newton solve");

  return params;
}

SinglePhaseFluidProperties::SinglePhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters),
    // downstream apps are creating fluid properties without their parameters, hence the workaround
    _tolerance(isParamValid("tolerance") ? getParam<Real>("tolerance") : 1e-8),
    _T_initial_guess(isParamValid("T_initial_guess") ? getParam<Real>("T_initial_guess") : 400),
    _p_initial_guess(isParamValid("p_initial_guess") ? getParam<Real>("p_initial_guess") : 2e5),
    _max_newton_its(getParam<unsigned int>("max_newton_its"))
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties() {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

Real
SinglePhaseFluidProperties::s_from_p_T(const Real pressure, const Real temperature) const
{
  Real v, e;
  v_e_from_p_T(pressure, temperature, v, e);
  return s_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::s_from_p_T(
    const Real pressure, const Real temperature, Real & s, Real & ds_dp, Real & ds_dT) const
{
  Real v, e, dv_dp, dv_dT, de_dp, de_dT;
  v_e_from_p_T(pressure, temperature, v, dv_dp, dv_dT, e, de_dp, de_dT);

  Real ds_dv, ds_de;
  s_from_v_e(v, e, s, ds_dv, ds_de);
  ds_dp = ds_dv * dv_dp + ds_de * de_dp;
  ds_dT = ds_dv * dv_dT + ds_de * de_dT;
}

Real
SinglePhaseFluidProperties::s_from_v_e(const Real v, const Real e) const
{
  const Real p0 = _p_initial_guess;
  const Real T0 = _T_initial_guess;
  Real p, T;
  bool conversion_succeeded = true;
  p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
  const Real s = s_from_p_T(p, T);
  return s;
}

void
SinglePhaseFluidProperties::s_from_v_e(
    const Real v, const Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  const Real p0 = _p_initial_guess;
  const Real T0 = _T_initial_guess;
  Real p, T;
  bool conversion_succeeded = true;
  p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
  s = s_from_p_T(p, T);
  ds_dv = p / T;
  ds_de = 1 / T;
}

Real
SinglePhaseFluidProperties::c_from_p_T(Real p, Real T) const
{
  Real v, e;
  v_e_from_p_T(p, T, v, e);
  return c_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::c_from_p_T(Real p, Real T, Real & c, Real & dc_dp, Real & dc_dT) const
{
  Real v, e, dv_dp, dv_dT, de_dp, de_dT;
  v_e_from_p_T(p, T, v, dv_dp, dv_dT, e, de_dp, de_dT);

  Real dc_dv, dc_de;
  c_from_v_e(v, e, c, dc_dv, dc_de);
  dc_dp = dc_dv * dv_dp + dc_de * de_dp;
  dc_dT = dc_dv * dv_dT + dc_de * de_dT;
}

Real
SinglePhaseFluidProperties::mu_from_p_T(Real p, Real T) const
{
  Real v, e;
  v_e_from_p_T(p, T, v, e);
  return mu_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::mu_from_p_T(
    Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real v, e, dv_dp, dv_dT, de_dp, de_dT;
  v_e_from_p_T(p, T, v, dv_dp, dv_dT, e, de_dp, de_dT);

  Real dmu_dv, dmu_de;
  mu_from_v_e(v, e, mu, dmu_dv, dmu_de);
  dmu_dp = dmu_dv * dv_dp + dmu_de * de_dp;
  dmu_dT = dmu_dv * dv_dT + dmu_de * de_dT;
}

Real
SinglePhaseFluidProperties::cv_from_p_T(Real p, Real T) const
{
  Real v, e;
  v_e_from_p_T(p, T, v, e);
  return cv_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::cv_from_p_T(
    Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  Real v, e, dv_dp, dv_dT, de_dp, de_dT;
  v_e_from_p_T(p, T, v, dv_dp, dv_dT, e, de_dp, de_dT);

  Real dcv_dv, dcv_de;
  cv_from_v_e(v, e, cv, dcv_dv, dcv_de);
  dcv_dp = dcv_dv * dv_dp + dcv_de * de_dp;
  dcv_dT = dcv_dv * dv_dT + dcv_de * de_dT;
}

Real
SinglePhaseFluidProperties::cp_from_p_T(Real p, Real T) const
{
  Real v, e;
  v_e_from_p_T(p, T, v, e);
  return cp_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::cp_from_p_T(
    Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  Real v, e, dv_dp, dv_dT, de_dp, de_dT;
  v_e_from_p_T(p, T, v, dv_dp, dv_dT, e, de_dp, de_dT);

  Real dcp_dv, dcp_de;
  cp_from_v_e(v, e, cp, dcp_dv, dcp_de);
  dcp_dp = dcp_dv * dv_dp + dcp_de * de_dp;
  dcp_dT = dcp_dv * dv_dT + dcp_de * de_dT;
}

Real
SinglePhaseFluidProperties::k_from_p_T(Real p, Real T) const
{
  Real v, e;
  v_e_from_p_T(p, T, v, e);
  return k_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  Real v, e, dv_dp, dv_dT, de_dp, de_dT;
  v_e_from_p_T(p, T, v, dv_dp, dv_dT, e, de_dp, de_dT);

  Real dk_dv, dk_de;
  k_from_v_e(v, e, k, dk_dv, dk_de);
  dk_dp = dk_dv * dv_dp + dk_de * de_dp;
  dk_dT = dk_dv * dv_dT + dk_de * de_dT;
}

Real
SinglePhaseFluidProperties::h_from_v_e(Real v, Real e) const
{
  return e + v * p_from_v_e(v, e);
}

void
SinglePhaseFluidProperties::h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const
{
  Real p, dp_dv, dp_de;
  p_from_v_e(v, e, p, dp_dv, dp_de);
  h = e + v * p;
  dh_dv = p + v * dp_dv;
  dh_de = 1 + v * dp_de;
}

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
  mooseError(__PRETTY_FUNCTION__, " is not implemented.");
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
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

std::string
SinglePhaseFluidProperties::fluidName() const
{
  return std::string("");
}

Real
SinglePhaseFluidProperties::criticalPressure() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::criticalTemperature() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::criticalDensity() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::criticalInternalEnergy() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::triplePointPressure() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

Real
SinglePhaseFluidProperties::triplePointTemperature() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
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
  unimplementedDerivativeMethod(__PRETTY_FUNCTION__);

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
  unimplementedDerivativeMethod(__PRETTY_FUNCTION__);

  dgamma_dp = 0.0;
  dgamma_dT = 0.0;
  gamma = gamma_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::vaporPressure(Real) const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

std::vector<Real>
SinglePhaseFluidProperties::henryCoefficients() const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::vaporPressure(Real T, Real & p, Real & dp_dT) const
{
  unimplementedDerivativeMethod(__PRETTY_FUNCTION__);

  dp_dT = 0.0;
  p = vaporPressure(T);
}

ADReal
SinglePhaseFluidProperties::vaporPressure(const ADReal & T) const
{
  Real p = 0.0;
  Real temperature = T.value();
  Real dpdT = 0.0;

  vaporPressure(temperature, p, dpdT);

  ADReal result = p;
  result.derivatives() = T.derivatives() * dpdT;

  return result;
}

Real
SinglePhaseFluidProperties::vaporTemperature(Real) const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::vaporTemperature(Real p, Real & T, Real & dT_dp) const
{
  unimplementedDerivativeMethod(__PRETTY_FUNCTION__);

  dT_dp = 0.0;
  T = vaporTemperature(p);
}

ADReal
SinglePhaseFluidProperties::vaporTemperature(const ADReal & p) const
{
  Real T = 0.0;
  Real pressure = p.value();
  Real dTdp = 0.0;

  vaporTemperature(pressure, T, dTdp);

  ADReal result = T;
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
SinglePhaseFluidProperties::rho_mu_from_p_T(const ADReal & p,
                                            const ADReal & T,
                                            ADReal & rho,
                                            ADReal & mu) const
{
  rho = rho_from_p_T(p, T);
  mu = mu_from_p_T(p, T);
}

Real
SinglePhaseFluidProperties::e_spndl_from_v(Real) const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
}

void
SinglePhaseFluidProperties::v_e_spndl_from_T(Real, Real &, Real &) const
{
  mooseError(__PRETTY_FUNCTION__, " not implemented.");
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

Real
SinglePhaseFluidProperties::p_from_h_s(Real h, Real s) const
{
  Real p0 = _p_initial_guess;
  Real T0 = _T_initial_guess;
  Real p, T;
  bool conversion_succeeded = true;
  p_T_from_h_s(h, s, p0, T0, p, T, conversion_succeeded);
  return p;
}

void
SinglePhaseFluidProperties::p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const
{
  Real p0 = _p_initial_guess;
  Real T0 = _T_initial_guess;
  Real T;
  bool conversion_succeeded = true;
  p_T_from_h_s(h, s, p0, T0, p, T, conversion_succeeded);
  dp_dh = rho_from_p_T(p, T);
  dp_ds = -T * rho_from_p_T(p, T);
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

#pragma GCC diagnostic pop
