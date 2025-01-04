//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemperaturePressureFunctionFluidProperties.h"
#include "NewtonInversion.h"

registerMooseObject("FluidPropertiesApp", TemperaturePressureFunctionFluidProperties);

InputParameters
TemperaturePressureFunctionFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addRequiredParam<FunctionName>(
      "k", "Thermal conductivity function of temperature and pressure [W/(m-K)]");
  params.addRequiredParam<FunctionName>("rho",
                                        "Density function of temperature and pressure [kg/m^3]");
  params.addRequiredParam<FunctionName>(
      "mu", "Dynamic viscosity function of temperature and pressure [Pa-s]");

  params.addParam<FunctionName>(
      "cp", "Isobaric specific heat function of temperature and pressure [J/(kg-K)]");
  params.addRangeCheckedParam<Real>(
      "cv", 0, "cv >= 0", "Constant isochoric specific heat [J/(kg-K)]");
  params.addParam<Real>("e_ref", 0, "Specific energy at the reference temperature");
  params.addParam<Real>("T_ref", 0, "Reference temperature for the specific energy");

  params.addClassDescription(
      "Single-phase fluid properties that allows to provide thermal "
      "conductivity, density, and viscosity as functions of temperature and pressure.");
  return params;
}

TemperaturePressureFunctionFluidProperties::TemperaturePressureFunctionFluidProperties(
    const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _initialized(false),
    _cv(getParam<Real>("cv")),
    _cv_is_constant(_cv != 0),
    _e_ref(getParam<Real>("e_ref")),
    _T_ref(getParam<Real>("T_ref"))
{
  if (isParamValid("cp") && _cv_is_constant)
    paramError("cp", "Can only set a function for cp if a constant is not specified for cv.");
}

void
TemperaturePressureFunctionFluidProperties::initialSetup()
{
  _k_function = &getFunction("k");
  _rho_function = &getFunction("rho");
  _mu_function = &getFunction("mu");
  _cp_function = isParamValid("cp") ? &getFunction("cp") : nullptr;
  _initialized = true;
}

std::string
TemperaturePressureFunctionFluidProperties::fluidName() const
{
  return "TemperaturePressureFunctionFluidProperties";
}

Real
TemperaturePressureFunctionFluidProperties::T_from_v_e(Real v, Real e) const
{
  if (_cv_is_constant)
    return _T_ref + (e - _e_ref) / _cv;
  else
  {
    const Real p0 = _p_initial_guess;
    const Real T0 = _T_initial_guess;
    Real p, T;
    bool conversion_succeeded = true;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    if (conversion_succeeded)
      return T;
    else
      mooseError("T_from_v_e calculation failed.");
  }
}

Real
TemperaturePressureFunctionFluidProperties::T_from_p_h(Real p, Real h) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, current_T, new_h, dh_dp, dh_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               p, h, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h", _max_newton_its)
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               p,
               ") and enthalpy (h = ",
               h,
               ") to temperature failed to converge.");
  return T;
}

Real
TemperaturePressureFunctionFluidProperties::T_from_p_rho(Real p, Real rho) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_rho, Real & drho_dp, Real & drho_dT)
  { rho_from_p_T(p, current_T, new_rho, drho_dp, drho_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               p, rho, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_rho")
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               p,
               ") and density (rho = ",
               rho,
               ") to temperature failed to converge.");
  return T;
}

Real
TemperaturePressureFunctionFluidProperties::cp_from_v_e(Real v, Real e) const
{
  if (_cv_is_constant)
  {
    Real p = p_from_v_e(v, e);
    Real T = T_from_v_e(v, e);
    return cp_from_p_T(p, T);
  }
  else
  {
    const Real p0 = _p_initial_guess;
    const Real T0 = _T_initial_guess;
    Real p, T;
    bool conversion_succeeded = true;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    if (conversion_succeeded)
      return cp_from_p_T(p, T);
    else
      mooseError("cp_from_v_e calculation failed. p= ", p, " T = ", T);
  }
}

void
TemperaturePressureFunctionFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  // Using finite difference to get around difficulty of implementation
  Real eps = 1e-10;
  Real cp_pert = cp_from_v_e(v * (1 + eps), e);
  dcp_dv = (cp_pert - cp) / eps / v;
  cp_pert = cp_from_v_e(v, e * (1 + eps));
  dcp_de = (cp_pert - cp) / eps / e;
}

Real
TemperaturePressureFunctionFluidProperties::cv_from_v_e(Real v, Real e) const
{
  if (_cv_is_constant)
    return _cv;
  else
  {
    const Real p0 = _p_initial_guess;
    const Real T0 = _T_initial_guess;
    Real p, T;
    bool conversion_succeeded = true;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    if (conversion_succeeded)
      return cv_from_p_T(p, T);
    else
      mooseError("cp_from_v_e calculation failed.");
  }
}

void
TemperaturePressureFunctionFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  if (_cv_is_constant)
  {
    cv = cv_from_v_e(v, e);
    dcv_dv = 0.0;
    dcv_de = 0.0;
  }
  else
  {
    const Real p0 = _p_initial_guess;
    const Real T0 = _T_initial_guess;
    Real p, T;
    bool conversion_succeeded = true;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    Real dcv_dp, dcv_dT;
    cv_from_p_T(p, T, cv, dcv_dp, dcv_dT);
    if (!conversion_succeeded)
      mooseError("cp_from_v_e and derivatives calculation failed.");

    Real p1, T1;
    p_T_from_v_e(v * (1 + 1e-6), e, p0, T0, p1, T1, conversion_succeeded);
    Real dp_dv = (p1 - p) / (v * 1e-6);
    Real dT_dv = (T1 - T) / (v * 1e-6);
    if (!conversion_succeeded)
      mooseError("cp_from_v_e and derivatives calculation failed.");

    Real p2, T2;
    p_T_from_v_e(v, e * (1 + 1e-6), p0, T0, p2, T2, conversion_succeeded);
    Real dp_de = (p2 - p) / (e * 1e-6);
    Real dT_de = (T2 - T) / (e * 1e-6);
    if (!conversion_succeeded)
      mooseError("cp_from_v_e and derivatives calculation failed.");

    dcv_dv = dcv_dp * dp_dv + dcv_dT * dT_dv;
    dcv_de = dcv_dp * dp_de + dcv_dT * dT_de;
  }
}

Real
TemperaturePressureFunctionFluidProperties::p_from_v_e(Real v, Real e) const
{
  const Real T = T_from_v_e(v, e);
  // note that p and T inversion in the definition of lambda
  auto lambda = [&](Real T, Real current_p, Real & new_rho, Real & drho_dT, Real & drho_dp)
  { rho_from_p_T(current_p, T, new_rho, drho_dp, drho_dT); };
  Real p = FluidPropertiesUtils::NewtonSolve(
               T, 1. / v, _p_initial_guess, _tolerance, lambda, name() + "::p_from_v_e")
               .first;
  // check for nans
  if (std::isnan(p))
    mooseError("Conversion from specific volume (v = ",
               v,
               ") and specific energy (e = ",
               e,
               ") to pressure failed to converge.");
  return p;
}

Real
TemperaturePressureFunctionFluidProperties::mu_from_v_e(Real v, Real e) const
{
  if (_cv_is_constant)
  {
    Real temperature = T_from_v_e(v, e);
    Real pressure = p_from_v_e(v, e);
    return mu_from_p_T(pressure, temperature);
  }
  else
  {
    const Real p0 = _p_initial_guess;
    const Real T0 = _T_initial_guess;
    Real p, T;
    bool conversion_succeeded = true;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    if (conversion_succeeded)
      return mu_from_p_T(p, T);
    else
      mooseError("mu_from_v_e calculation failed.");
  }
}

Real
TemperaturePressureFunctionFluidProperties::k_from_v_e(Real v, Real e) const
{
  if (_cv_is_constant)
  {
    Real temperature = T_from_v_e(v, e);
    Real pressure = p_from_v_e(v, e);
    return k_from_p_T(pressure, temperature);
  }
  else
  {
    const Real p0 = _p_initial_guess;
    const Real T0 = _T_initial_guess;
    Real p, T;
    bool conversion_succeeded = true;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    if (conversion_succeeded)
      return k_from_p_T(p, T);
    else
      mooseError("k_from_v_e calculation failed.");
  }
}

Real
TemperaturePressureFunctionFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  if (!_initialized)
    const_cast<TemperaturePressureFunctionFluidProperties *>(this)->initialSetup();
  return _rho_function->value(0, Point(temperature, pressure, 0));
}

void
TemperaturePressureFunctionFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  const RealVectorValue grad_function = _rho_function->gradient(0, Point(temperature, pressure, 0));
  drho_dT = grad_function(0);
  drho_dp = grad_function(1);
}

void
TemperaturePressureFunctionFluidProperties::rho_from_p_T(const ADReal & pressure,
                                                         const ADReal & temperature,
                                                         ADReal & rho,
                                                         ADReal & drho_dp,
                                                         ADReal & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  const ADRealVectorValue grad_function =
      _rho_function->gradient(0, Point(temperature.value(), pressure.value(), 0));
  drho_dT = grad_function(0);
  drho_dp = grad_function(1);
}

Real
TemperaturePressureFunctionFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  return 1.0 / rho_from_p_T(pressure, temperature);
}

void
TemperaturePressureFunctionFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(pressure, temperature);

  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);

  dv_dp = -v * v * drho_dp;
  dv_dT = -v * v * drho_dT;
}

Real
TemperaturePressureFunctionFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  Real e = e_from_p_T(pressure, temperature);
  Real rho = rho_from_p_T(pressure, temperature);
  return e + pressure / rho;
}

void
TemperaturePressureFunctionFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  Real e, de_dp, de_dT;
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  h = e + pressure / rho;
  dh_dp = de_dp + 1 / rho - pressure / rho / rho * drho_dp;
  dh_dT = de_dT - pressure * drho_dT / rho / rho;
}

Real
TemperaturePressureFunctionFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  if (_cv_is_constant)
    return _e_ref + _cv * (temperature - _T_ref);
  else
  {
    // Use Simpson's 3/8 rule to integrate cv
    Real h = (temperature - _T_ref) / 3;
    Real integral =
        3. / 8. * h *
        (cv_from_p_T(pressure, _T_ref) + 3 * cv_from_p_T(pressure, _T_ref + h) +
         3 * cv_from_p_T(pressure, _T_ref + 2 * h) + cv_from_p_T(pressure, temperature));
    return _e_ref + integral;
  }
}

void
TemperaturePressureFunctionFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  if (_cv_is_constant)
  {
    e = e_from_p_T(pressure, temperature);
    de_dp = 0.0;
    de_dT = _cv;
  }
  else
  {
    e = e_from_p_T(pressure, temperature);
    // Use Simpson's 3/8 rule to integrate cv then take the derivative
    Real h = (temperature - _T_ref) / 3;
    Real cv0, dcv_dp0, dcv_dT0;
    cv_from_p_T(pressure, _T_ref, cv0, dcv_dp0, dcv_dT0);
    Real cv1, dcv_dp1, dcv_dT1;
    cv_from_p_T(pressure, _T_ref + h, cv1, dcv_dp1, dcv_dT1);
    Real cv2, dcv_dp2, dcv_dT2;
    cv_from_p_T(pressure, _T_ref + 2 * h, cv2, dcv_dp2, dcv_dT2);
    Real cv3, dcv_dp3, dcv_dT3;
    cv_from_p_T(pressure, temperature, cv3, dcv_dp3, dcv_dT3);
    de_dp = 3. / 8 * h * (dcv_dp0 + 3 * dcv_dp1 + 3 * dcv_dp2 + dcv_dp3);
    // Consistency with how the energy is computed is more important here
    de_dT =
        1. / 8. * (cv0 + 3 * cv1 + 3 * cv2 + cv3) + 3. / 8 * h * (dcv_dT1 + 2 * dcv_dT2 + dcv_dT3);
  }
}

Real
TemperaturePressureFunctionFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  if (_cv_is_constant)
  {
    Real temperature = T_from_p_rho(p, rho);
    return _e_ref + _cv * (temperature - _T_ref);
  }
  else
  {
    Real temperature = T_from_p_rho(p, rho);
    return e_from_p_T(p, temperature);
  }
}

Real
TemperaturePressureFunctionFluidProperties::beta_from_p_T(Real pressure, Real temperature) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  return -drho_dp / rho;
}

Real
TemperaturePressureFunctionFluidProperties::cp_from_p_T(Real p, Real T) const
{
  if (_cv_is_constant)
  {
    Real rho, drho_dp, drho_dT;
    rho_from_p_T(p, T, rho, drho_dp, drho_dT);
    Real alpha = -1. / rho * drho_dT;
    return _cv + MathUtils::pow(alpha, 2) * T / rho / beta_from_p_T(p, T);
  }
  else
  {
    if (!_initialized)
      const_cast<TemperaturePressureFunctionFluidProperties *>(this)->initialSetup();
    return _cp_function->value(0, Point(T, p, 0));
  }
}

void
TemperaturePressureFunctionFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  if (_cv_is_constant)
  {
    cp = cp_from_p_T(pressure, temperature);
    Real eps = 1e-8;
    Real cp_1p = cp_from_p_T(pressure * (1 + eps), temperature);
    Real cp_1T = cp_from_p_T(pressure, temperature * (1 + eps));
    dcp_dp = (cp_1p - cp) / (pressure * eps);
    dcp_dT = (cp_1T - cp) / (temperature * eps);
  }
  else
  {
    cp = cp_from_p_T(pressure, temperature);
    const RealVectorValue grad_function =
        _cp_function->gradient(0, Point(temperature, pressure, 0));
    dcp_dT = grad_function(0);
    dcp_dp = grad_function(1);
  }
}

Real
TemperaturePressureFunctionFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  if (_cv_is_constant)
    return _cv;
  else
  {
    Real rho, drho_dp, drho_dT;
    rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
    Real alpha = -1. / rho * drho_dT;
    return cp_from_p_T(pressure, temperature) -
           MathUtils::pow(alpha, 2) * temperature / rho / beta_from_p_T(pressure, temperature);
  }
}

void
TemperaturePressureFunctionFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  if (_cv_is_constant)
  {
    cv = cv_from_p_T(pressure, temperature);
    dcv_dp = 0.0;
    dcv_dT = 0.0;
  }
  else
  {
    cv = cv_from_p_T(pressure, temperature);
    Real eps = 1e-10;
    Real cv_1p = cv_from_p_T(pressure * (1 + eps), temperature);
    Real cv_1T = cv_from_p_T(pressure, temperature * (1 + eps));
    dcv_dp = (cv_1p - cv) / (pressure * eps);
    dcv_dT = (cv_1T - cv) / (temperature * eps);
  }
}

Real
TemperaturePressureFunctionFluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  if (!_initialized)
    const_cast<TemperaturePressureFunctionFluidProperties *>(this)->initialSetup();
  return _mu_function->value(0, Point(temperature, pressure, 0));
}

void
TemperaturePressureFunctionFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(pressure, temperature);
  const RealVectorValue grad_function = _mu_function->gradient(0, Point(temperature, pressure, 0));
  dmu_dT = grad_function(0);
  dmu_dp = grad_function(1);
}

Real
TemperaturePressureFunctionFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  if (!_initialized)
    const_cast<TemperaturePressureFunctionFluidProperties *>(this)->initialSetup();
  return _k_function->value(0, Point(temperature, pressure, 0));
}

void
TemperaturePressureFunctionFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(pressure, temperature);
  const RealVectorValue grad_function = _k_function->gradient(0, Point(temperature, pressure, 0));
  dk_dT = grad_function(0);
  dk_dp = grad_function(1);
}
