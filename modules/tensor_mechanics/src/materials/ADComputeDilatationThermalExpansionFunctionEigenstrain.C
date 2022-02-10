//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeDilatationThermalExpansionFunctionEigenstrain.h"

#include "Function.h"

registerMooseObject("TensorMechanicsApp", ADComputeDilatationThermalExpansionFunctionEigenstrain);

InputParameters
ADComputeDilatationThermalExpansionFunctionEigenstrain::validParams()
{
  InputParameters params = ADComputeDilatationThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the total dilatation as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "dilatation_function",
      "Function describing the thermal dilatation as a function of temperature");
  return params;
}

ADComputeDilatationThermalExpansionFunctionEigenstrain::
    ADComputeDilatationThermalExpansionFunctionEigenstrain(const InputParameters & parameters)
  : ADComputeDilatationThermalExpansionEigenstrainBase(parameters),
    _dilatation_function(getFunction("dilatation_function"))
{
}

ADReal
ADComputeDilatationThermalExpansionFunctionEigenstrain::computeDilatation(
    const ADReal & temperature)
{
  return _dilatation_function.value(temperature);
}
