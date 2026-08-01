//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SodiumSaturationFluidProperties.h"
#include "NewtonInversion.h"

registerMooseObject("FluidPropertiesApp", SodiumSaturationFluidProperties);

InputParameters
SodiumSaturationFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for liquid sodium at saturation conditions");
  return params;
}

SodiumSaturationFluidProperties::SodiumSaturationFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

std::string
SodiumSaturationFluidProperties::fluidName() const
{
  return "sodium_sat";
}

Real
SodiumSaturationFluidProperties::molarMass() const
{
  return 22.989769E-3;
}

Real
SodiumSaturationFluidProperties::rho_from_p_T(Real /* pressure */, Real temperature) const
{
  return 1.00423e3 - 0.21390 * temperature - 1.1046e-5 * temperature * temperature;
}

void
SodiumSaturationFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0.0;
  drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
}

void
SodiumSaturationFluidProperties::rho_from_p_T(const ADReal & pressure,
                                              const ADReal & temperature,
                                              ADReal & rho,
                                              ADReal & drho_dp,
                                              ADReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(pressure, temperature);
  drho_dp = 0.0;
  drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
}

Real
SodiumSaturationFluidProperties::rho_from_p_s(Real pressure, Real entropy) const
{
  auto entropy_from_p_T = [&](Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT)
  { s_from_p_T(p, T, s, ds_dp, ds_dT); };
  const Real temperature = FluidPropertiesUtils::NewtonSolve(pressure,
                                                             entropy,
                                                             _T_initial_guess,
                                                             _tolerance,
                                                             entropy_from_p_T,
                                                             name() + "::rho_from_p_s",
                                                             _max_newton_its,
                                                             _verbose_newton)
                               .first;
  return rho_from_p_T(pressure, temperature);
}

void
SodiumSaturationFluidProperties::rho_from_p_s(
    Real pressure, Real entropy, Real & rho, Real & drho_dp, Real & drho_ds) const
{
  auto entropy_from_p_T = [&](Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT)
  { s_from_p_T(p, T, s, ds_dp, ds_dT); };
  const Real temperature = FluidPropertiesUtils::NewtonSolve(pressure,
                                                             entropy,
                                                             _T_initial_guess,
                                                             _tolerance,
                                                             entropy_from_p_T,
                                                             name() + "::rho_from_p_s",
                                                             _max_newton_its,
                                                             _verbose_newton)
                               .first;

  Real s, ds_dp, ds_dT;
  s_from_p_T(pressure, temperature, s, ds_dp, ds_dT);
  const Real dT_dp = -ds_dp / ds_dT;
  const Real dT_ds = 1 / ds_dT;

  Real drho_dp_T, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp_T, drho_dT);
  drho_dp = drho_dp_T + drho_dT * dT_dp;
  drho_ds = drho_dT * dT_ds;
}

Real
SodiumSaturationFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  return 1.0 / rho_from_p_T(pressure, temperature);
}

void
SodiumSaturationFluidProperties::specific_volume_derivatives(
    Real temperature, Real & v, Real & dv_dT, Real & d2v_dT2, Real & d3v_dT3) const
{
  const Real rho = rho_from_p_T(_reference_pressure, temperature);
  const Real drho_dT = -0.21390 - 2 * 1.1046e-5 * temperature;
  const Real d2rho_dT2 = -2 * 1.1046e-5;

  v = 1.0 / rho;
  dv_dT = -drho_dT / (rho * rho);
  d2v_dT2 = 2 * drho_dT * drho_dT / (rho * rho * rho) - d2rho_dT2 / (rho * rho);
  d3v_dT3 = 6 * drho_dT * d2rho_dT2 / (rho * rho * rho) -
            6 * drho_dT * drho_dT * drho_dT / (rho * rho * rho * rho);
}

Real
SodiumSaturationFluidProperties::cp0_from_T(Real temperature) const
{
  const Real t2 = temperature * temperature;
  return 3.7782E-10 * t2 * t2 - 1.7191E-6 * t2 * temperature + 3.0921E-3 * t2 -
         2.4560 * temperature + 1972.0;
}

Real
SodiumSaturationFluidProperties::dcp0_dT_from_T(Real temperature) const
{
  const Real t2 = temperature * temperature;
  return 4 * 3.7782E-10 * t2 * temperature - 3 * 1.7191E-6 * t2 + 2 * 3.0921e-3 * temperature -
         2.456;
}

void
SodiumSaturationFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(pressure, temperature);
  dv_dp = 0.0;

  Real drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
  dv_dT = -v * v * drho_dT;
}

Real
SodiumSaturationFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  Real dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);

  const Real h = h_from_p_T(_reference_pressure, temperature);
  return _reference_pressure + (h - _reference_pressure * v - e) / (temperature * dv_dT);
}

void
SodiumSaturationFluidProperties::p_from_v_e(
    Real v, Real e, Real & pressure, Real & dp_dv, Real & dp_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  Real dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);

  const Real h = h_from_p_T(_reference_pressure, temperature);
  const Real cp = cp0_from_T(temperature);
  const Real numerator = h - _reference_pressure * v - e;
  const Real denominator = temperature * dv_dT;
  const Real dnumerator_dv = (cp - _reference_pressure * dv_dT) * dT_dv;
  const Real ddenominator_dv = (dv_dT + temperature * d2v_dT2) * dT_dv;

  pressure = _reference_pressure + numerator / denominator;
  dp_dv = (dnumerator_dv * denominator - numerator * ddenominator_dv) / (denominator * denominator);
  dp_de = -1.0 / denominator;
}

Real
SodiumSaturationFluidProperties::T_from_v_e(Real v, Real /* e */) const
{
  // From inversion of second order polynomial form of rho(T)
  mooseAssert(0.2139 * 0.2139 + 4 * 1.1046e5 * (1.00423e3 - 1 / v) > 0,
              "Specific volume out of bounds");
  return (0.2139 - std::sqrt(0.2139 * 0.2139 + 4 * 1.1046e-5 * (1.00423e3 - 1 / v))) /
         (2 * -1.1046e-5);
}

void
SodiumSaturationFluidProperties::T_from_v_e(
    Real v, Real e, Real & temperature, Real & dT_dv, Real & dT_de) const
{
  temperature = T_from_v_e(v, e);
  const Real drho_dT = -0.21390 - 1.1046e-5 * 2 * temperature;
  dT_dv = -1 / (v * v * drho_dT);
  dT_de = 0;
}

Real
SodiumSaturationFluidProperties::c_from_v_e(Real v, Real e) const
{
  const Real temperature = T_from_v_e(v, e);
  // Equation (3) in the saturated-sodium speed-of-sound section of Fink and Leibowitz (1979),
  // valid from 370.98 K to 1173 K.
  return 2660.7 - 0.37667 * temperature - 9.0356e-5 * temperature * temperature;
}

void
SodiumSaturationFluidProperties::c_from_v_e(
    Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  c = c_from_v_e(v, e);
  const Real dc_dT = -0.37667 - 2 * 9.0356e-5 * temperature;
  dc_dv = dc_dT * dT_dv;
  dc_de = dc_dT * dT_de;
}

Real
SodiumSaturationFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  Real t2 = temperature * temperature;
  const Real h0 = 3.7782E-10 * t2 * t2 * temperature / 5 - 1.7191E-6 * t2 * t2 / 4.0 +
                  3.0921E-3 * t2 * temperature / 3.0 - 2.4560 * t2 / 2.0 + 1972.0 * temperature -
                  401088.7;

  Real v, dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);
  return h0 + (pressure - _reference_pressure) * (v - temperature * dv_dT);
}

void
SodiumSaturationFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);

  Real v, dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);
  dh_dp = v - temperature * dv_dT;
  dh_dT = cp0_from_T(temperature) - (pressure - _reference_pressure) * temperature * d2v_dT2;
}

Real
SodiumSaturationFluidProperties::T_from_p_h(Real pressure, Real enthalpy) const
{
  auto enthalpy_from_p_T = [&](Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, T, h, dh_dp, dh_dT); };

  return FluidPropertiesUtils::NewtonSolve(pressure,
                                           enthalpy,
                                           _T_initial_guess,
                                           _tolerance,
                                           enthalpy_from_p_T,
                                           name() + "::T_from_p_h",
                                           _max_newton_its,
                                           _verbose_newton)
      .first;
}

void
SodiumSaturationFluidProperties::T_from_p_h(
    Real pressure, Real enthalpy, Real & temperature, Real & dT_dp, Real & dT_dh) const
{
  temperature = T_from_p_h(pressure, enthalpy);

  Real h, dh_dp, dh_dT;
  h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
  dT_dp = -dh_dp / dh_dT;
  dT_dh = 1.0 / dh_dT;
}

Real
SodiumSaturationFluidProperties::s_from_p_T(Real pressure, Real temperature) const
{
  // Use a first-order pressure extension consistent with the Gibbs relation for v=v(T).
  constexpr Real reference_temperature = 370.98;
  const auto temperature_part = [](Real T)
  {
    const Real T2 = T * T;
    return 3.7782e-10 * T2 * T2 / 4.0 - 1.7191e-6 * T2 * T / 3.0 + 3.0921e-3 * T2 / 2.0 -
           2.4560 * T + 1972.0 * std::log(T);
  };

  Real v, dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);
  return temperature_part(temperature) - temperature_part(reference_temperature) -
         (pressure - _reference_pressure) * dv_dT;
}

void
SodiumSaturationFluidProperties::s_from_p_T(
    Real pressure, Real temperature, Real & s, Real & ds_dp, Real & ds_dT) const
{
  s = s_from_p_T(pressure, temperature);

  Real v, dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);
  ds_dp = -dv_dT;
  ds_dT = cp_from_p_T(pressure, temperature) / temperature;
}

Real
SodiumSaturationFluidProperties::s_from_v_e(Real v, Real e) const
{
  return s_from_p_T(p_from_v_e(v, e), T_from_v_e(v, e));
}

void
SodiumSaturationFluidProperties::s_from_v_e(
    Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  Real pressure, dp_dv, dp_de;
  p_from_v_e(v, e, pressure, dp_dv, dp_de);

  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  Real ds_dp, ds_dT;
  s_from_p_T(pressure, temperature, s, ds_dp, ds_dT);
  ds_dv = ds_dp * dp_dv + ds_dT * dT_dv;
  ds_de = ds_dp * dp_de + ds_dT * dT_de;
}

Real
SodiumSaturationFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  // definition of h = e + p * v
  Real v = v_from_p_T(pressure, temperature);
  Real h = h_from_p_T(pressure, temperature);
  return h - pressure * v;
}

void
SodiumSaturationFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  Real v, dv_dp, dv_dT;
  v_from_p_T(pressure, temperature, v, dv_dp, dv_dT);
  Real h, dh_dp, dh_dT;
  h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
  e = h - pressure * v;

  // definition of e = h - p * v
  de_dp = dh_dp - v - pressure * dv_dp;

  de_dT = dh_dT - pressure * dv_dT;
}

Real
SodiumSaturationFluidProperties::e_from_p_rho(Real pressure, Real rho) const
{
  return e_from_p_T(pressure, T_from_v_e(1 / rho, 0));
}

void
SodiumSaturationFluidProperties::e_from_p_rho(
    Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  const Real v = 1 / rho;
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, 0, temperature, dT_dv, dT_de);

  Real de_dp_T, de_dT;
  e_from_p_T(pressure, temperature, e, de_dp_T, de_dT);
  de_dp = de_dp_T;
  de_drho = de_dT * dT_dv * -v * v;
}

Real
SodiumSaturationFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  Real v, dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);
  return cp0_from_T(temperature) - (pressure - _reference_pressure) * temperature * d2v_dT2;
}

void
SodiumSaturationFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  Real v, dv_dT, d2v_dT2, d3v_dT3;
  specific_volume_derivatives(temperature, v, dv_dT, d2v_dT2, d3v_dT3);
  dcp_dp = -temperature * d2v_dT2;
  dcp_dT = dcp0_dT_from_T(temperature) -
           (pressure - _reference_pressure) * (d2v_dT2 + temperature * d3v_dT3);
}

Real
SodiumSaturationFluidProperties::cp_from_v_e(Real v, Real e) const
{
  return cp_from_p_T(p_from_v_e(v, e), T_from_v_e(v, e));
}

void
SodiumSaturationFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real pressure, dp_dv, dp_de;
  p_from_v_e(v, e, pressure, dp_dv, dp_de);
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  Real dcp_dp, dcp_dT;
  cp_from_p_T(pressure, temperature, cp, dcp_dp, dcp_dT);
  dcp_dv = dcp_dp * dp_dv + dcp_dT * dT_dv;
  dcp_de = dcp_dp * dp_de + dcp_dT * dT_de;
}

Real
SodiumSaturationFluidProperties::cv_from_p_T(Real /* pressure */, Real temperature) const
{
  Real t2 = temperature * temperature;
  return 1.0369E-8 * temperature * t2 + 3.7164E-4 * t2 - 1.0494 * temperature + 1582.6;
}

void
SodiumSaturationFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 3 * 1.0369e-8 * temperature * temperature + 2 * 3.7164e-4 * temperature - 1.0494;
}

Real
SodiumSaturationFluidProperties::cv_from_v_e(Real v, Real e) const
{
  return cv_from_p_T(p_from_v_e(v, e), T_from_v_e(v, e));
}

void
SodiumSaturationFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  Real pressure, dp_dv, dp_de;
  p_from_v_e(v, e, pressure, dp_dv, dp_de);
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  Real dcv_dp, dcv_dT;
  cv_from_p_T(pressure, temperature, cv, dcv_dp, dcv_dT);
  dcv_dv = dcv_dp * dp_dv + dcv_dT * dT_dv;
  dcv_de = dcv_dp * dp_de + dcv_dT * dT_de;
}

Real
SodiumSaturationFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 3.6522E-5 + 0.16626 / temperature - 4.56877e1 / (temperature * temperature) +
         2.8733E4 / (temperature * temperature * temperature);
}

void
SodiumSaturationFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(pressure, temperature);
  dmu_dp = 0.0;

  Real t2 = temperature * temperature;
  dmu_dT = 0.16626 * -1 / t2 - 4.56877E1 * -2 / (temperature * t2) + 2.8733E4 * -3 / (t2 * t2);
}

Real
SodiumSaturationFluidProperties::mu_from_v_e(Real v, Real e) const
{
  return mu_from_p_T(p_from_v_e(v, e), T_from_v_e(v, e));
}

void
SodiumSaturationFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  Real pressure, dp_dv, dp_de;
  p_from_v_e(v, e, pressure, dp_dv, dp_de);
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  Real dmu_dp, dmu_dT;
  mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
  dmu_dv = dmu_dp * dp_dv + dmu_dT * dT_dv;
  dmu_de = dmu_dp * dp_de + dmu_dT * dT_de;
}

Real
SodiumSaturationFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  return 1.1045e2 - 6.5112e-2 * temperature + 1.5430e-5 * temperature * temperature -
         2.4617e-9 * temperature * temperature * temperature;
}

void
SodiumSaturationFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = this->k_from_p_T(pressure, temperature);
  dk_dp = 0.0;
  dk_dT = -6.5112e-2 + 2 * 1.5430e-5 * temperature - 3 * 2.4617e-9 * temperature * temperature;
}

Real
SodiumSaturationFluidProperties::k_from_v_e(Real v, Real e) const
{
  return k_from_p_T(p_from_v_e(v, e), T_from_v_e(v, e));
}

void
SodiumSaturationFluidProperties::k_from_v_e(
    Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  Real pressure, dp_dv, dp_de;
  p_from_v_e(v, e, pressure, dp_dv, dp_de);
  Real temperature, dT_dv, dT_de;
  T_from_v_e(v, e, temperature, dT_dv, dT_de);

  Real dk_dp, dk_dT;
  k_from_p_T(pressure, temperature, k, dk_dp, dk_dT);
  dk_dv = dk_dp * dp_dv + dk_dT * dT_dv;
  dk_de = dk_dp * dp_de + dk_dT * dT_de;
}
