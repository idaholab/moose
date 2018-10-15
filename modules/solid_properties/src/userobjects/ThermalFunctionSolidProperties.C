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
  InputParameters params = validParams<ThermalSolidProperties>();
  params.addClassDescription("Userobject defining thermal properties from functions.");
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity");
  params.addRequiredParam<FunctionName>("cp", "Isobaric specific heat");
  params.addRequiredParam<FunctionName>("rho", "Density");
  return params;
}

ThermalFunctionSolidProperties::ThermalFunctionSolidProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
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
ThermalFunctionSolidProperties::cp_from_T(Real T) const
{
  return _cp.value(T, Point());
}

void
ThermalFunctionSolidProperties::cp_from_T(Real T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);

  // Because we parameterize time as temperature, the derivative with respect to
  // temperature is actually computed as a time derivative
  dcp_dT = _cp.timeDerivative(T, Point());
}

Real
ThermalFunctionSolidProperties::k_from_T(Real T) const
{
  return _k.value(T, Point());
}

void
ThermalFunctionSolidProperties::k_from_T(Real T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);

  // Because we parameterize time as temperature, the derivative with respect to
  // temperature is actually computed as a time derivative
  dk_dT = _k.timeDerivative(T, Point());
}

Real
ThermalFunctionSolidProperties::rho_from_T(Real T) const
{
  return _rho.value(T, Point());
}

void
ThermalFunctionSolidProperties::rho_from_T(Real T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);

  // Because we parameterize time as temperature, the derivative with respect to
  // temperature is actually computed as a time derivative
  drho_dT = _rho.timeDerivative(T, Point());
}
