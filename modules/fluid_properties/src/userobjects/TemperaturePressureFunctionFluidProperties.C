//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemperaturePressureFunctionFluidProperties.h"

registerMooseObject("FluidPropertiesApp", TemperaturePressureFunctionFluidProperties);

InputParameters
TemperaturePressureFunctionFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity function");
  params.addRequiredParam<FunctionName>("rho", "Density function");
  params.addRequiredParam<FunctionName>("mu", "Dynamic viscosity function");

  params.addRequiredRangeCheckedParam<Real>("cp", "cp > 0", "Constant isobaric specific heat");
  params.addRequiredRangeCheckedParam<Real>("cv", "cv > 0", "Constant isochoric specific heat");
  params.addClassDescription(
      "Single phase fluid properties that allows to provide thermal "
      "conductivity, density, and viscosity as function of temperature and pressure.");
  return params;
}

TemperaturePressureFunctionFluidProperties::TemperaturePressureFunctionFluidProperties(
    const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _cp(getParam<Real>("cp")), _cv(getParam<Real>("cv"))
{
}

void
TemperaturePressureFunctionFluidProperties::initialSetup()
{
  _k_function = &getFunction("k");
  _rho_function = &getFunction("rho");
  _mu_function = &getFunction("mu");
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
TemperaturePressureFunctionFluidProperties::T_from_p_h(Real /* p */, Real h) const
{
  return h / _cp;
}

Real TemperaturePressureFunctionFluidProperties::T_from_p_rho(Real /* p */, Real /* rho */) const
{
  mooseError("not implemented");
  return 0;
}

Real TemperaturePressureFunctionFluidProperties::cp_from_v_e(Real /*v*/, Real /*e*/) const
{
  return _cp;
}

Real TemperaturePressureFunctionFluidProperties::cv_from_v_e(Real /* v */, Real /* e */) const
{
  return _cv;
}

Real
TemperaturePressureFunctionFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real temperature = T_from_v_e(v, e);
  Real enthalpy = h_from_p_T(0, temperature);
  return (enthalpy - e) / v;
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
  const RealVectorValue grad_function =
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
TemperaturePressureFunctionFluidProperties::h_from_p_T(Real /* pressure */, Real temperature) const
{
  return _cp * temperature;
}

void
TemperaturePressureFunctionFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(pressure, temperature);
  dh_dp = 0.0;
  dh_dT = _cp;
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

Real TemperaturePressureFunctionFluidProperties::cp_from_p_T(Real /* pressure */,
                                                             Real /* temperature */) const
{
  return _cp;
}

void
TemperaturePressureFunctionFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0.0;
  dcp_dT = 0.0;
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
