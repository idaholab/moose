//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalFunctionSolidProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalFunctionSolidProperties);

const std::string ThermalFunctionSolidProperties::_name = "thermal_function";

template <>
InputParameters
validParams<ThermalFunctionSolidProperties>()
{
  InputParameters params = validParams<ThermalSolidPropertiesMaterial>();
  params.addClassDescription("Userobject defining thermal properties from functions.");
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity");
  params.addRequiredParam<FunctionName>("cp", "Isobaric specific heat");
  params.addRequiredParam<FunctionName>("rho", "Density");
  return params;
}

ThermalFunctionSolidProperties::ThermalFunctionSolidProperties(const InputParameters & parameters)
  : ThermalSolidPropertiesMaterial(parameters),
    _k(getFunction("k")),
    _cp(getFunction("cp")),
    _rho(getFunction("rho"))
{
}

const std::string &
ThermalFunctionSolidProperties::solidName() const
{
  return _name;
}

Real
ThermalFunctionSolidProperties::cp() const
{
  return _cp.value(_temperature[_qp], Point());
  //  dcp_dT = _cp.timeDerivative(T, Point());
}

Real
ThermalFunctionSolidProperties::k() const
{
  return _k.value(_temperature[_qp], Point());
  //   dk_dT = _k.timeDerivative(T, Point());
}

Real
ThermalFunctionSolidProperties::rho() const
{
  return _rho.value(_temperature[_qp], Point());
  //   drho_dT = _rho.timeDerivative(T, Point());
}

Real
ThermalFunctionSolidProperties::beta() const
{
  Real drho_dT = _rho.timeDerivative(_temperature[_qp], Point());
  return -drho_dT / rho();
}
