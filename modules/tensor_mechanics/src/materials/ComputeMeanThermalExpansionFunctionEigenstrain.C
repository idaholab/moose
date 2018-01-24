//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMeanThermalExpansionFunctionEigenstrain.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeMeanThermalExpansionFunctionEigenstrain>()
{
  InputParameters params = validParams<ComputeMeanThermalExpansionEigenstrainBase>();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the mean thermal expansion as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "thermal_expansion_function",
      "Function describing the mean thermal expansion as a function of temperature");
  params.addRequiredParam<Real>("thermal_expansion_function_reference_temperature",
                                "Reference temperature for thermal_exansion_function (IMPORTANT: "
                                "this is different in general from the stress_free_temperature)");

  return params;
}

ComputeMeanThermalExpansionFunctionEigenstrain::ComputeMeanThermalExpansionFunctionEigenstrain(
    const InputParameters & parameters)
  : ComputeMeanThermalExpansionEigenstrainBase(parameters),
    _thermal_expansion_function(getFunction("thermal_expansion_function")),
    _thexp_func_ref_temp(getParam<Real>("thermal_expansion_function_reference_temperature"))
{
}

Real
ComputeMeanThermalExpansionFunctionEigenstrain::referenceTemperature()
{
  return _thexp_func_ref_temp;
}

Real
ComputeMeanThermalExpansionFunctionEigenstrain::meanThermalExpansionCoefficient(
    const Real temperature)
{
  return _thermal_expansion_function.value(temperature, Point());
}

Real
ComputeMeanThermalExpansionFunctionEigenstrain::meanThermalExpansionCoefficientDerivative(
    const Real temperature)
{
  return _thermal_expansion_function.timeDerivative(temperature, Point());
}
