//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalFunctionSolidProperties.h"
#include "Function.h"

registerMooseObject("SolidPropertiesApp", ThermalFunctionSolidProperties);

InputParameters
ThermalFunctionSolidProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity");
  params.addRequiredParam<FunctionName>("cp", "Isobaric specific heat");
  params.addRequiredParam<FunctionName>("rho", "Density");
  params.addClassDescription("Function-based thermal properties.");
  return params;
}

ThermalFunctionSolidProperties::ThermalFunctionSolidProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _k_function(getFunction("k")),
    _cp_function(getFunction("cp")),
    _rho_function(getFunction("rho"))
{
}

Real
ThermalFunctionSolidProperties::cp_from_T(const Real & T) const
{
  return _cp_function.value(T);
}

void
ThermalFunctionSolidProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = _cp_function.timeDerivative(T);
}

Real
ThermalFunctionSolidProperties::k_from_T(const Real & T) const
{
  return _k_function.value(T);
}

void
ThermalFunctionSolidProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);
  dk_dT = _k_function.timeDerivative(T);
}

Real
ThermalFunctionSolidProperties::rho_from_T(const Real & T) const
{
  return _rho_function.value(T);
}

void
ThermalFunctionSolidProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = _rho_function.timeDerivative(T);
}
