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

InputParameters
ThermalFunctionSolidProperties::validParams()
{
  InputParameters params = ThermalSolidPropertiesMaterial::validParams();
  params.addClassDescription("Userobject defining thermal properties from functions.");
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity");
  params.addRequiredParam<FunctionName>("cp", "Isobaric specific heat");
  params.addRequiredParam<FunctionName>("rho", "Density");
  return params;
}

ThermalFunctionSolidProperties::ThermalFunctionSolidProperties(const InputParameters & parameters)
  : ThermalSolidPropertiesMaterial(parameters),
    _k_function(getFunction("k")),
    _cp_function(getFunction("cp")),
    _rho_function(getFunction("rho"))
{
}

const std::string &
ThermalFunctionSolidProperties::solidName() const
{
  return _name;
}

void
ThermalFunctionSolidProperties::computeIsobaricSpecificHeat()
{
  _cp[_qp] = _cp_function.value(_temperature[_qp], Point());
}

void
ThermalFunctionSolidProperties::computeIsobaricSpecificHeatDerivatives()
{
  _dcp_dT[_qp] = _cp_function.timeDerivative(_temperature[_qp], Point());
}

void
ThermalFunctionSolidProperties::computeThermalConductivity()
{
  _k[_qp] = _k_function.value(_temperature[_qp], Point());
}

void
ThermalFunctionSolidProperties::computeThermalConductivityDerivatives()
{
  _dk_dT[_qp] = _k_function.timeDerivative(_temperature[_qp], Point());
}

void
ThermalFunctionSolidProperties::computeDensity()
{
  _rho[_qp] = _rho_function.value(_temperature[_qp], Point());
}

void
ThermalFunctionSolidProperties::computeDensityDerivatives()
{
  _drho_dT[_qp] = _rho_function.timeDerivative(_temperature[_qp], Point());
}
