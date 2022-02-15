//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDilatationThermalExpansionFunctionEigenstrain.h"

#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeDilatationThermalExpansionFunctionEigenstrain);

InputParameters
ComputeDilatationThermalExpansionFunctionEigenstrain::validParams()
{
  InputParameters params = ComputeDilatationThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the total dilatation as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "dilatation_function",
      "Function describing the thermal dilatation as a function of temperature");
  return params;
}

ComputeDilatationThermalExpansionFunctionEigenstrain::
    ComputeDilatationThermalExpansionFunctionEigenstrain(const InputParameters & parameters)
  : ComputeDilatationThermalExpansionEigenstrainBase(parameters),
    _dilatation_function(getFunction("dilatation_function"))
{
}

Real
ComputeDilatationThermalExpansionFunctionEigenstrain::computeDilatation(const Real & temperature)
{
  return _dilatation_function.value(temperature, Point());
}

Real
ComputeDilatationThermalExpansionFunctionEigenstrain::computeDilatationDerivative(
    const Real & temperature)
{
  return _dilatation_function.timeDerivative(temperature);
}
