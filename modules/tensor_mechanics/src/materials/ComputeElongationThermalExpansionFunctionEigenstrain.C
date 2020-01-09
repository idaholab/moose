//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeElongationThermalExpansionFunctionEigenstrain.h"

registerMooseObject("TensorMechanicsApp", ComputeElongationThermalExpansionFunctionEigenstrain);

defineLegacyParams(ComputeElongationThermalExpansionFunctionEigenstrain);

InputParameters
ComputeElongationThermalExpansionFunctionEigenstrain::validParams()
{
  InputParameters params = ComputeElongationThermalExpansionEigenstrainBase::validParams();
  params.addRequiredParam<FunctionName>(
      "elongation_function",
      "Function describing the thermal elongation as a function of temperature");
  return params;
}

ComputeElongationThermalExpansionFunctionEigenstrain::
    ComputeElongationThermalExpansionFunctionEigenstrain(const InputParameters & parameters)
  : ComputeElongationThermalExpansionEigenstrainBase(parameters),
    _elongation_function(getFunction("elongation_function"))
{
}

Real
ComputeElongationThermalExpansionFunctionEigenstrain::computeElongation(const Real & temperature)
{
  return _elongation_function.value(temperature, Point());
}

Real
ComputeElongationThermalExpansionFunctionEigenstrain::computeElongationDerivative(
    const Real & temperature)
{
  return _elongation_function.timeDerivative(temperature, Point());
}
