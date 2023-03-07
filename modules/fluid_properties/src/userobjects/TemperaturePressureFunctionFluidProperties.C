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

  params.addRequiredRangeCheckedParam<Real>(
      "cv", "cv > 0", "Constant isochoric specific heat [J/(kg-K)]");
  params.addClassDescription(
      "Single-phase fluid properties that allows to provide thermal "
      "conductivity, density, and viscosity as functions of temperature and pressure.");
  return params;
}

TemperaturePressureFunctionFluidProperties::TemperaturePressureFunctionFluidProperties(
    const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _initialized(false), _cv(getParam<Real>("cv"))
{
}

void
TemperaturePressureFunctionFluidProperties::initialSetup()
{
  _k_function = &getFunction("k");
  _rho_function = &getFunction("rho");
  _mu_function = &getFunction("mu");
  _initialized = true;
}

std::string
TemperaturePressureFunctionFluidProperties::fluidName() const
{
  return "TemperaturePressureFunctionFluidProperties";
}

Real
TemperaturePressureFunctionFluidProperties::T_from_v_e(Real /* v */, Real e) const
{
  return e / _cv;
}

Real
TemperaturePressureFunctionFluidProperties::T_from_p_h(Real p, Real h) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, current_T, new_h, dh_dp, dh_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               p, h, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h")
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
  Real p = p_from_v_e(v, e);
  Real T = T_from_v_e(v, e);
  return cp_from_p_T(p, T);
}

void
TemperaturePressureFunctionFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  // Using finite difference to get around difficulty of implementation
  Real eps = 1e-8;
  Real cp_pert = cp_from_v_e(v * (1 + eps), e);
  dcp_dv = (cp_pert - cp) / eps / v;
  cp_pert = cp_from_v_e(v, e * (1 + eps));
  dcp_de = (cp_pert - cp) / eps / e;
}

Real TemperaturePressureFunctionFluidProperties::cv_from_v_e(Real /* v */, Real /* e */) const
{
  return _cv;
}

void
TemperaturePressureFunctionFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = 0.0;
  dcv_de = 0.0;
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
  Real temperature = T_from_v_e(v, e);
  Real pressure = p_from_v_e(v, e);
  return mu_from_p_T(pressure, temperature);
}

Real
TemperaturePressureFunctionFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  Real pressure = p_from_v_e(v, e);
  return k_from_p_T(pressure, temperature);
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
TemperaturePressureFunctionFluidProperties::rho_from_p_T(const DualReal & pressure,
                                                         const DualReal & temperature,
                                                         DualReal & rho,
                                                         DualReal & drho_dp,
                                                         DualReal & drho_dT) const
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
TemperaturePressureFunctionFluidProperties::e_from_p_T(Real /* pressure */, Real temperature) const
{
  return _cv * temperature;
}

void
TemperaturePressureFunctionFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(pressure, temperature);
  de_dp = 0.0;
  de_dT = _cv;
}

Real
TemperaturePressureFunctionFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  Real temperature = T_from_p_rho(p, rho);
  return _cv * temperature;
}

Real
TemperaturePressureFunctionFluidProperties::beta_from_p_T(Real pressure, Real temperature) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  return -drho_dT / rho;
}

Real
TemperaturePressureFunctionFluidProperties::cp_from_p_T(Real p, Real T) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real dv_dT = -1 / rho / rho * drho_dT;
  // neglecting dp_dT term due to difficulty in computing it in the general case
  // this is not OK for gases, but should be ok for nearly incompressible fluids
  return _cv + p * dv_dT;
  // an alternative would be to use finite differencing for the p * v term in its entirety
}

void
TemperaturePressureFunctionFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  Real e, de_dp, de_dT;
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  Real dv_dT = -1 / rho / rho * drho_dT;
  cp = _cv + pressure * dv_dT;

  // use finite difference for second derivative to avoid having to define hessian for the function
  Real eps = 1e-8;
  Real rho_pert, drho_dp_pert, drho_dT_pert;
  rho_from_p_T(pressure, temperature * (1 + eps), rho_pert, drho_dp_pert, drho_dT_pert);
  Real d2v_dT2 =
      -(-1 / rho / rho * drho_dT + 1 / rho_pert / rho_pert * drho_dT_pert) / eps / temperature;
  rho_from_p_T(pressure * (1 + eps), temperature, rho_pert, drho_dp_pert, drho_dT_pert);
  Real d2v_dTdp =
      -(-1 / rho / rho * drho_dT + 1 / rho_pert / rho_pert * drho_dT_pert) / eps / pressure;

  dcp_dp = dv_dT + pressure * d2v_dTdp;
  dcp_dT = pressure * d2v_dT2;
}

Real TemperaturePressureFunctionFluidProperties::cv_from_p_T(Real /* pressure */,
                                                             Real /* temperature */) const
{
  return _cv;
}

void
TemperaturePressureFunctionFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  cv = cv_from_p_T(pressure, temperature);
  dcv_dp = 0.0;
  dcv_dT = 0.0;
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
