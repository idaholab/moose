//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CaloricallyImperfectGas.h"
#include "Conversion.h"
#include "metaphysicl/raw_type.h"
#include "Function.h"
#include "BrentsMethod.h"

registerMooseObject("FluidPropertiesApp", CaloricallyImperfectGas);

InputParameters
CaloricallyImperfectGas::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params += NaNInterface::validParams();

  params.addRequiredParam<Real>("molar_mass", "Constant molar mass of the fluid [kg/mol]");
  params.addRequiredParam<FunctionName>("mu", "Dynamic viscosity, [Pa-s]");
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity, [W/(m-K)]");
  params.addParam<Real>("T_c", 0, "Critical temperature, [K]");
  params.addParam<Real>("rho_c", 0, "Critical density, [kg/m3]");
  params.addParam<Real>("e_c", 0, "Specific internal energy at the critical point, [J/kg]");
  params.addRequiredParam<FunctionName>(
      "e", "Specific internal energy as a function of temperature [J/kg]");
  params.addRequiredParam<Real>("min_temperature", "Smallest temperature for lookup tables [K]");
  params.addRequiredParam<Real>("max_temperature", "Largest temperature for lookup tables [K]");
  params.addParam<Real>(
      "temperature_resolution", 1, "Step size in temperature for creating the inverse lookup T(e)");
  params.addParam<bool>("out_of_bound_error", true, "Error if out of bounds");
  params.addClassDescription("Fluid properties for an ideal gas with imperfect caloric behavior.");

  return params;
}

CaloricallyImperfectGas::CaloricallyImperfectGas(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    NaNInterface(this),
    _molar_mass(getParam<Real>("molar_mass")),
    _R_specific(_R / _molar_mass),
    _T_c(getParam<Real>("T_c")),
    _rho_c(getParam<Real>("rho_c")),
    _e_c(getParam<Real>("e_c")),
    _min_temperature(getParam<Real>("min_temperature")),
    _max_temperature(getParam<Real>("max_temperature")),
    _delta_T(getParam<Real>("temperature_resolution")),
    _out_of_bound_error(getParam<bool>("out_of_bound_error")),
    _tol(1e-4)
{
}

void
CaloricallyImperfectGas::initialSetup()
{
  SinglePhaseFluidProperties::initialSetup();
  _e_T = &getFunction("e");
  _mu_T = &getFunction("mu");
  _k_T = &getFunction("k");

  setupLookupTables();
}

void
CaloricallyImperfectGas::setupLookupTables()
{
  // estimate number of points in inverse lookup & then adjust
  // the _delta_T so (n - 1) * _delta_T = _max_temperature - _min_temperature
  unsigned int n = std::floor((_max_temperature - _min_temperature) / _delta_T) + 1;
  _delta_T = (_max_temperature - _min_temperature) / ((Real)n - 1.0);

  // ensure e(T) is monotonic
  for (unsigned int j = 0; j < n; ++j)
  {
    Real temperature = _min_temperature + j * _delta_T;

    // Note that the function behavior at the end points
    // can lead to de/dT = 0 & we want to allow that
    Real cv = _e_T->timeDerivative(temperature, Point());
    if (cv < 0 || (cv == 0 && j > 0 && j < n - 1))
      mooseError("e(T) is not monotonically increasing with T");
  }

  // backward lookup tables
  _min_e = _e_T->value(_min_temperature, Point());
  _max_e = _e_T->value(_max_temperature, Point());
  _delta_e = (_max_e - _min_e) / ((Real)n - 1.0);
  _min_h = _min_e + _R_specific * _min_temperature;
  _max_h = _max_e + _R_specific * _max_temperature;
  _delta_h = (_max_h - _min_h) / ((Real)n - 1.0);

  _T_e_lookup.resize(n);
  _T_h_lookup.resize(n);
  _Z_T_lookup.resize(n);
  for (unsigned int j = 0; j < n; ++j)
  {
    // internal energy
    {
      Real internal_energy = _min_e + j * _delta_e;

      // guarding against roundoff when summing
      // at least once I saw roundoff cause an error here
      internal_energy = std::min(internal_energy, _max_e);

      // The temperature is found by e - e(T) = 0
      Real min = _min_temperature;
      Real max = _max_temperature;
      auto e_diff = [&internal_energy, this](Real x)
      { return this->e_from_p_T(0.0, x) - internal_energy; };
      BrentsMethod::bracket(e_diff, min, max);
      Real temperature = BrentsMethod::root(e_diff, min, max);
      _T_e_lookup[j] = temperature;
    }

    // enthalpy
    {
      Real enthalpy = _min_h + j * _delta_h;

      // guarding against roundoff when summing
      // at least once I saw roundoff cause an error here
      enthalpy = std::min(enthalpy, _max_h);

      // The temperature is found by h - h(T) = 0
      Real min = _min_temperature;
      Real max = _max_temperature;
      auto h_diff = [&enthalpy, this](Real x) { return this->h_from_p_T(0.0, x) - enthalpy; };
      BrentsMethod::bracket(h_diff, min, max);
      Real temperature = BrentsMethod::root(h_diff, min, max);
      _T_h_lookup[j] = temperature;
    }

    // Z(T)
    {
      if (j == 0)
        _Z_T_lookup[j] = 0;
      else
      {
        Real temperature = _min_temperature + j * _delta_T;
        Real temperature_prev = _min_temperature + (j - 1) * _delta_T;
        Real f1 = cv_from_T(temperature) / temperature;
        Real f2 = cv_from_T(temperature_prev) / temperature_prev;
        _Z_T_lookup[j] = _Z_T_lookup[j - 1] + 0.5 * _delta_T * (f1 + f2);
      }
    }
  }
}

std::string
CaloricallyImperfectGas::fluidName() const
{
  return "caloric_imperfect_gas";
}

Real
CaloricallyImperfectGas::e_from_T(Real T) const
{
  if (T < _min_temperature || T > _max_temperature)
    outOfBounds("e_from_T", T, _min_temperature, _max_temperature);

  return _e_T->value(T, Point());
}

Real
CaloricallyImperfectGas::h_from_T(Real T) const
{
  if (T < _min_temperature || T > _max_temperature)
    outOfBounds("h_from_T", T, _min_temperature, _max_temperature);
  return _e_T->value(T, Point()) + _R_specific * T;
}

Real
CaloricallyImperfectGas::cp_from_T(Real T) const
{
  if (T < _min_temperature || T > _max_temperature)
    outOfBounds("cp_from_T", T, _min_temperature, _max_temperature);

  return _e_T->timeDerivative(T, Point()) + _R_specific;
}

void
CaloricallyImperfectGas::cv_from_T(Real T, Real & cv, Real & dcv_dT) const
{
  if (T < _min_temperature || T > _max_temperature)
    outOfBounds("cv_from_T (3 args)", T, _min_temperature, _max_temperature);
  Real pert = 1.0e-7;
  cv = cv_from_T(T);
  Real cv_pert = cv_from_T(T * (1 + pert));
  dcv_dT = (cv_pert - cv) / (T * pert);
}

void
CaloricallyImperfectGas::cp_from_T(Real T, Real & cp, Real & dcp_dT) const
{
  if (T < _min_temperature || T > _max_temperature)
    outOfBounds("cp_from_T (3 args)", T, _min_temperature, _max_temperature);
  Real pert = 1.0e-7;
  cp = cp_from_T(T);
  Real cp_pert = cp_from_T(T * (1 + pert));
  dcp_dT = (cp_pert - cp) / (T * pert);
}

Real
CaloricallyImperfectGas::T_from_e(Real e) const
{
  if (e < _min_e || e > _max_e)
    outOfBounds("T_from_e", e, _min_e, _max_e);

  if (e < _min_e)
    return _T_e_lookup[0];
  else if (e > _max_e)
    return _T_e_lookup.back();

  unsigned int index = std::floor((e - _min_e) / _delta_e);
  Real x = (e - _min_e - index * _delta_e) / _delta_e;
  return x * _T_e_lookup[index + 1] + (1 - x) * _T_e_lookup[index];
}

Real
CaloricallyImperfectGas::T_from_h(Real h) const
{
  if (h < _min_h || h > _max_h)
    outOfBounds("h_from_e", h, _min_h, _max_h);

  if (h < _min_h)
    return _T_h_lookup[0];
  else if (h > _max_h)
    return _T_h_lookup.back();

  unsigned int index = std::floor((h - _min_h) / _delta_h);
  Real x = (h - _min_h - index * _delta_h) / _delta_h;
  return x * _T_h_lookup[index + 1] + (1 - x) * _T_h_lookup[index];
}

Real
CaloricallyImperfectGas::Z_from_T(Real T) const
{
  if (T < _min_temperature || T > _max_temperature)
    outOfBounds("Z_from_T", T, _min_temperature, _max_temperature);

  unsigned int index = std::floor((T - _min_temperature) / _delta_T);
  Real x = (T - _min_temperature - index * _delta_T) / _delta_T;
  return x * _Z_T_lookup[index + 1] + (1 - x) * _Z_T_lookup[index];
}

Real
CaloricallyImperfectGas::p_from_v_e(Real v, Real e) const
{
  if (v == 0.0)
    return getNaN("Invalid value of specific volume detected (v = " + Moose::stringify(v) + ").");

  return _R_specific * T_from_e(e) / v;
}

ADReal
CaloricallyImperfectGas::p_from_v_e(const ADReal & v, const ADReal & e) const
{
  if (v.value() == 0.0)
    return getNaN("Invalid value of specific volume detected (v = " + Moose::stringify(v.value()) +
                  ").");

  Real x = 0;
  Real raw1 = v.value();
  Real raw2 = e.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  p_from_v_e(raw1, raw2, x, dxd1, dxd2);

  DualReal result = x;
  result.derivatives() = v.derivatives() * dxd1 + e.derivatives() * dxd2;
  return result;
}

void
CaloricallyImperfectGas::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  Real T = T_from_e(e);
  dp_dv = -_R_specific * T / v / v;
  dp_de = _R_specific / v / cv_from_T(T);
}

Real
CaloricallyImperfectGas::T_from_v_e(Real /*v*/, Real e) const
{
  return T_from_e(e);
}

ADReal
CaloricallyImperfectGas::T_from_v_e(const ADReal & v, const ADReal & e) const
{
  if (v.value() == 0.0)
    return getNaN("Invalid value of specific volume detected (v = " + Moose::stringify(v.value()) +
                  ").");

  Real x = 0;
  Real raw1 = v.value();
  Real raw2 = e.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  T_from_v_e(raw1, raw2, x, dxd1, dxd2);

  DualReal result = x;
  result.derivatives() = v.derivatives() * dxd1 + e.derivatives() * dxd2;
  return result;
}

void
CaloricallyImperfectGas::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_e(e);
  dT_dv = 0.0;
  dT_de = 1.0 / cv_from_v_e(v, e);
}

Real
CaloricallyImperfectGas::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);

  const Real c2 = gamma_from_v_e(v, e) * _R_specific * T;
  if (c2 < 0)
    return getNaN("Sound speed squared (gamma * R * T) is negative: c2 = " + Moose::stringify(c2) +
                  ".");

  return std::sqrt(c2);
}

ADReal
CaloricallyImperfectGas::c_from_v_e(const ADReal & v, const ADReal & e) const
{
  const auto T = T_from_v_e(v, e);

  const auto c2 = gamma_from_v_e(v, e) * _R_specific * T;
  if (MetaPhysicL::raw_value(c2) < 0)
    return getNaN("Sound speed squared (gamma * R * T) is negative: c2 = " +
                  Moose::stringify(MetaPhysicL::raw_value(c2)) + ".");

  return std::sqrt(c2);
}

void
CaloricallyImperfectGas::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);

  Real gamma, dgamma_dv, dgamma_dT;
  gamma_from_v_e(v, e, gamma, dgamma_dv, dgamma_dT);
  c = std::sqrt(gamma * _R_specific * T);

  const Real dc_dT = 0.5 / c * _R_specific * (gamma + T * dgamma_dT);
  dc_dv = dc_dT * dT_dv;
  dc_de = dc_dT * dT_de;
}

Real
CaloricallyImperfectGas::c_from_p_T(Real p, Real T) const
{
  return std::sqrt(gamma_from_p_T(p, T) * _R_specific * T);
}

ADReal
CaloricallyImperfectGas::c_from_p_T(const ADReal & p, const ADReal & T) const
{
  return std::sqrt(gamma_from_p_T(p, T) * _R_specific * T);
}

void
CaloricallyImperfectGas::c_from_p_T(
    const Real p, const Real T, Real & c, Real & dc_dp, Real & dc_dT) const
{
  Real gamma, dgamma_dp, dgamma_dT;
  gamma_from_p_T(p, T, gamma, dgamma_dp, dgamma_dT);
  c = std::sqrt(gamma * _R_specific * T);
  dc_dp = 0;
  dc_dT = 0.5 / c * _R_specific * (gamma + T * dgamma_dT);
}

Real
CaloricallyImperfectGas::cp_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return cp_from_T(T);
}

void
CaloricallyImperfectGas::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real T = T_from_v_e(v, e);
  Real dcp_dT;
  cp_from_T(T, cp, dcp_dT);
  dcp_dv = 0.0;
  dcp_de = dcp_dT / cv_from_T(T);
}

Real
CaloricallyImperfectGas::cv_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return cv_from_T(T);
}

ADReal
CaloricallyImperfectGas::cv_from_v_e(ADReal v, ADReal e) const
{
  Real x = 0;
  Real raw1 = v.value();
  Real raw2 = e.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  cv_from_v_e(raw1, raw2, x, dxd1, dxd2);

  DualReal result = x;
  result.derivatives() = v.derivatives() * dxd1 + e.derivatives() * dxd2;
  return result;
}

void
CaloricallyImperfectGas::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  Real T = T_from_v_e(v, e);
  Real dcv_dT;
  cv_from_T(T, cv, dcv_dT);
  dcv_dv = 0.0;
  dcv_de = dcv_dT / cv;
}

Real
CaloricallyImperfectGas::gamma_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e) / cv_from_v_e(v, e);
}

ADReal
CaloricallyImperfectGas::gamma_from_v_e(ADReal v, ADReal e) const
{
  Real x = 0;
  Real raw1 = v.value();
  Real raw2 = e.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  gamma_from_v_e(raw1, raw2, x, dxd1, dxd2);

  DualReal result = x;
  result.derivatives() = v.derivatives() * dxd1 + e.derivatives() * dxd2;
  return result;
}

void
CaloricallyImperfectGas::gamma_from_v_e(
    Real v, Real e, Real & gamma, Real & dgamma_dv, Real & dgamma_de) const
{
  Real cp, dcp_dv, dcp_de;
  cp_from_v_e(v, e, cp, dcp_dv, dcp_de);
  Real cv, dcv_dv, dcv_de;
  cv_from_v_e(v, e, cv, dcv_dv, dcv_de);
  gamma = cp / cv;
  dgamma_dv = 0.0;
  dgamma_de = 1.0 / cv * dcp_de - gamma / cv * dcv_de;
}

void
CaloricallyImperfectGas::gamma_from_p_T(
    Real p, Real T, Real & gamma, Real & dgamma_dp, Real & dgamma_dT) const
{
  Real cp, dcp_dp, dcp_dT;
  cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
  Real cv, dcv_dp, dcv_dT;
  cv_from_p_T(p, T, cv, dcv_dp, dcv_dT);
  gamma = cp / cv;
  dgamma_dp = 0.0;
  dgamma_dT = 1.0 / cv * dcp_dT - gamma / cv * dcv_dT;
}

Real
CaloricallyImperfectGas::gamma_from_p_T(Real p, Real T) const
{
  return cp_from_p_T(p, T) / cv_from_p_T(p, T);
}

ADReal
CaloricallyImperfectGas::gamma_from_p_T(ADReal p, ADReal T) const
{
  Real x = 0;
  Real raw1 = p.value();
  Real raw2 = T.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  gamma_from_p_T(raw1, raw2, x, dxd1, dxd2);
  DualReal result = x;
  result.derivatives() = p.derivatives() * dxd1 + T.derivatives() * dxd2;
  return result;
}

Real
CaloricallyImperfectGas::mu_from_v_e(Real v, Real e) const
{
  const Real T = T_from_v_e(v, e);
  const Real p = p_from_v_e(v, e);
  return mu_from_p_T(p, T);
}

void
CaloricallyImperfectGas::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  const Real T = T_from_v_e(v, e);
  const Real p = p_from_v_e(v, e);
  Real dmu_dp, dmu_dT;
  mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  // only dmu_de = dmu/dT * dT/de = 1/cv * dmu/dT is nonzero
  // (dmu/dv)_e = 0 because e only depends on T and e is constant
  dmu_dv = 0.0;
  dmu_de = dmu_dT / cv_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::k_from_v_e(Real v, Real e) const
{
  const Real T = T_from_v_e(v, e);
  const Real p = p_from_v_e(v, e);
  return k_from_p_T(p, T);
}

void
CaloricallyImperfectGas::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  const Real T = T_from_v_e(v, e);
  const Real p = p_from_v_e(v, e);
  Real dk_dp, dk_dT;
  k_from_p_T(p, T, k, dk_dp, dk_dT);
  // only dk_de = dk/dT * dT/de = 1/cv * dk/dT is nonzero
  // (dk/dv)_e = 0 because e only depends on T and e is constant
  dk_dv = 0.0;
  dk_de = dk_dT / cv_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::s_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  Real Z = Z_from_T(T);
  return Z + _R_specific * std::log(v);
}

void
CaloricallyImperfectGas::s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  s = s_from_v_e(v, e);
  // see documentation for derivation
  ds_dv = _R_specific / v;
  ds_de = 1.0 / T_from_v_e(v, e);
}

Real
CaloricallyImperfectGas::s_from_p_T(Real p, Real T) const
{
  Real Z = Z_from_T(T);
  Real v = 1.0 / rho_from_p_T(p, T);
  return Z + _R_specific * std::log(v);
}

void
CaloricallyImperfectGas::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  s = s_from_p_T(p, T);
  ds_dp = -_R_specific / p;
  ds_dT = cp_from_p_T(p, T) / T;
}

Real
CaloricallyImperfectGas::s_from_h_p(Real h, Real p) const
{
  Real T = T_from_p_h(p, h);
  Real v = 1.0 / rho_from_p_T(p, T);
  Real Z = Z_from_T(T);
  return Z + _R_specific * std::log(v);
}

void
CaloricallyImperfectGas::s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const
{
  s = s_from_h_p(h, p);
  ds_dh = 1.0 / T_from_p_h(p, h);
  ds_dp = -_R_specific / p;
}

Real
CaloricallyImperfectGas::rho_from_p_s(Real p, Real s) const
{
  auto s_diff = [&p, &s, this](Real x) { return this->s_from_p_T(p, x) - s; };
  Real min = _min_temperature;
  Real max = _max_temperature;
  BrentsMethod::bracket(s_diff, min, max);
  Real T = BrentsMethod::root(s_diff, min, max);
  return rho_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::e_from_v_h(Real /*v*/, Real h) const
{
  Real T = T_from_h(h);
  return e_from_T(T);
}

void
CaloricallyImperfectGas::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  e = e_from_v_h(v, h);
  Real cv = cv_from_v_e(v, e);
  Real cp = cp_from_v_e(v, e);
  de_dv = 0.0;
  de_dh = cv / cp;
}

Real
CaloricallyImperfectGas::rho_from_p_T(Real p, Real T) const
{
  return p * _molar_mass / (_R * T);
}

ADReal
CaloricallyImperfectGas::rho_from_p_T(const ADReal & p, const ADReal & T) const
{
  return p * _molar_mass / (_R * T);
}

void
CaloricallyImperfectGas::rho_from_p_T(const DualReal & p,
                                      const DualReal & T,
                                      DualReal & rho,
                                      DualReal & drho_dp,
                                      DualReal & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = _molar_mass / (_R * T);
  drho_dT = -p * _molar_mass / (_R * T * T);
}

void
CaloricallyImperfectGas::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = _molar_mass / (_R * T);
  drho_dT = -p * _molar_mass / (_R * T * T);
}

Real
CaloricallyImperfectGas::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_rho_template(p, rho);
}

ADReal
CaloricallyImperfectGas::e_from_p_rho(const ADReal & p, const ADReal & rho) const
{
  return e_from_p_rho_template(p, rho);
}

void
CaloricallyImperfectGas::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e_from_p_rho_template(p, rho, e, de_dp, de_drho);
}

void
CaloricallyImperfectGas::e_from_p_rho(
    const ADReal & p, const ADReal & rho, ADReal & e, ADReal & de_dp, ADReal & de_drho) const
{
  e_from_p_rho_template(p, rho, e, de_dp, de_drho);
}

Real
CaloricallyImperfectGas::e_from_T_v(Real T, Real /*v*/) const
{
  return e_from_T(T);
}

void
CaloricallyImperfectGas::e_from_T_v(Real T, Real v, Real & e, Real & de_dT, Real & de_dv) const
{
  e = e_from_T_v(T, v);
  de_dT = cv_from_T_v(T, v);
  de_dv = 0.0;
}

ADReal
CaloricallyImperfectGas::e_from_T_v(const ADReal & T, const ADReal & v) const
{
  Real x = 0;
  Real raw1 = T.value();
  Real raw2 = v.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  e_from_T_v(raw1, raw2, x, dxd1, dxd2);

  DualReal result = x;
  result.derivatives() = T.derivatives() * dxd1 + v.derivatives() * dxd2;
  return result;
}

Real
CaloricallyImperfectGas::p_from_T_v(Real T, Real v) const
{
  return _R_specific * T / v;
}

void
CaloricallyImperfectGas::p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const
{
  p = _R_specific * T / v;
  dp_dT = _R_specific / v;
  dp_dv = -_R_specific * T / (v * v);
}

Real
CaloricallyImperfectGas::h_from_T_v(Real T, Real /*v*/) const
{
  return h_from_T(T);
}

void
CaloricallyImperfectGas::h_from_T_v(Real T, Real /*v*/, Real & h, Real & dh_dT, Real & dh_dv) const
{
  h = h_from_T(T);
  dh_dT = cp_from_T(T);
  dh_dv = 0.0;
}

Real
CaloricallyImperfectGas::s_from_T_v(Real T, Real v) const
{
  Real p = p_from_T_v(T, v);
  return s_from_p_T(p, T);
}

void
CaloricallyImperfectGas::s_from_T_v(Real T, Real v, Real & s, Real & ds_dT, Real & ds_dv) const
{
  Real p, dp_dT_v, dp_dv_T;
  Real ds_dp_T, ds_dT_p;
  p_from_T_v(T, v, p, dp_dT_v, dp_dv_T);
  s_from_p_T(p, T, s, ds_dp_T, ds_dT_p);
  ds_dT = ds_dT_p + ds_dp_T * dp_dT_v;
  ds_dv = ds_dp_T * dp_dv_T;
}

Real
CaloricallyImperfectGas::cv_from_T_v(Real T, Real /*v*/) const
{
  return cv_from_T(T);
}

Real CaloricallyImperfectGas::e_spndl_from_v(Real /*v*/) const { return _e_c; }

void
CaloricallyImperfectGas::v_e_spndl_from_T(Real /*T*/, Real & v, Real & e) const
{
  v = 1. / _rho_c;
  e = _e_c;
}

Real
CaloricallyImperfectGas::h_from_p_T(Real /*p*/, Real T) const
{
  return h_from_T(T);
}

void
CaloricallyImperfectGas::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  dh_dp = 0.0;
  dh_dT = cp_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::e_from_p_T(Real /*p*/, Real T) const
{
  return e_from_T(T);
}

ADReal
CaloricallyImperfectGas::e_from_p_T(ADReal p, ADReal T) const
{
  Real x = 0;
  Real raw1 = p.value();
  Real raw2 = T.value();
  Real dxd1 = 0;
  Real dxd2 = 0;
  e_from_p_T(raw1, raw2, x, dxd1, dxd2);

  DualReal result = x;
  result.derivatives() = p.derivatives() * dxd1 + T.derivatives() * dxd2;
  return result;
}

void
CaloricallyImperfectGas::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(p, T);
  de_dp = 0.0;
  de_dT = cv_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::molarMass() const
{
  return _molar_mass;
}

Real
CaloricallyImperfectGas::criticalTemperature() const
{
  return _T_c;
}

Real
CaloricallyImperfectGas::criticalDensity() const
{
  return _rho_c;
}

Real
CaloricallyImperfectGas::criticalInternalEnergy() const
{
  return _e_c;
}

Real
CaloricallyImperfectGas::T_from_p_h(Real /*p*/, Real h) const
{
  return T_from_h(h);
}

void
CaloricallyImperfectGas::T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const
{
  T = T_from_p_h(p, h);
  dT_dp = 0;
  dT_dh = 1.0 / cp_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::cv_from_p_T(Real /* pressure */, Real temperature) const
{
  return cv_from_T(temperature);
}

void
CaloricallyImperfectGas::cv_from_p_T(
    Real /*p*/, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv_from_T(T, cv, dcv_dT);
  dcv_dp = 0.0;
}

Real
CaloricallyImperfectGas::cp_from_p_T(Real /* pressure */, Real temperature) const
{
  return cp_from_T(temperature);
}

void
CaloricallyImperfectGas::cp_from_p_T(
    Real /*p*/, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp_from_T(T, cp, dcp_dT);
  dcp_dp = 0.0;
}

Real
CaloricallyImperfectGas::mu_from_p_T(Real /* pressure */, Real temperature) const
{
  return _mu_T->value(temperature, Point());
}

void
CaloricallyImperfectGas::mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = this->mu_from_p_T(p, T);
  dmu_dp = 0.0;
  Real pert = 1.0e-7;
  Real mu2 = this->mu_from_p_T(p, T * (1 + pert));
  dmu_dT = (mu2 - mu) / (T * pert);
}

Real
CaloricallyImperfectGas::k_from_p_T(Real /* pressure */, Real temperature) const
{
  return _k_T->value(temperature, Point());
}

void
CaloricallyImperfectGas::k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(p, T);
  dk_dp = 0.0;
  Real pert = 1.0e-7;
  Real k2 = this->k_from_p_T(p, T * (1 + pert));
  dk_dT = (k2 - k) / (T * pert);
}

Real
CaloricallyImperfectGas::g_from_v_e(Real v, Real e) const
{
  // use the definition of the Gibbs free energy
  Real T = T_from_v_e(v, e);
  Real p = p_from_v_e(v, e);
  return h_from_p_T(p, T) - T * s_from_p_T(p, T);
}

Real
CaloricallyImperfectGas::p_from_h_s(Real h, Real s) const
{
  Real T = T_from_h(h);
  Real e = e_from_T(T);
  Real Z = Z_from_T(T);
  Real v = std::exp((s - Z) / _R_specific);
  return p_from_v_e(v, e);
}

void
CaloricallyImperfectGas::p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const
{
  p = p_from_h_s(h, s);
  Real pert = 1e-7;
  Real p_pert = p_from_h_s(h * (1 + pert), s);
  dp_dh = (p_pert - p) / (h * pert);
  p_pert = p_from_h_s(h, s * (1 + pert));
  dp_ds = (p_pert - p) / (s * pert);
}

void
CaloricallyImperfectGas::outOfBounds(const std::string & function,
                                     Real value,
                                     Real min,
                                     Real max) const
{
  std::stringstream ss;
  ss << "Function " << function << " encountered argument value of " << value
     << " which is outside of the bounds of " << min << " .. " << max;
  if (_out_of_bound_error)
    mooseError(ss.str());
  else
    mooseWarning(ss.str());
}
