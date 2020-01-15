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

registerADMooseObject("TensorMechanicsApp", ADComputeDilatationThermalExpansionFunctionEigenstrain);

defineADLegacyParams(ADComputeDilatationThermalExpansionFunctionEigenstrain);

template <ComputeStage compute_stage>
InputParameters
ADComputeDilatationThermalExpansionFunctionEigenstrain<compute_stage>::validParams()
{
  InputParameters params =
      ADComputeDilatationThermalExpansionEigenstrainBase<compute_stage>::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the total dilatation as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "dilatation_function",
      "Function describing the thermal dilatation as a function of temperature");
  return params;
}

template <ComputeStage compute_stage>
ADComputeDilatationThermalExpansionFunctionEigenstrain<compute_stage>::
    ADComputeDilatationThermalExpansionFunctionEigenstrain(const InputParameters & parameters)
  : ADComputeDilatationThermalExpansionEigenstrainBase<compute_stage>(parameters),
    _dilatation_function(getFunction("dilatation_function"))
{
}

template <ComputeStage compute_stage>
ADReal
ADComputeDilatationThermalExpansionFunctionEigenstrain<compute_stage>::computeDilatation(
    const ADReal & temperature)
{
  // Note: All dual number information will be lost until functions can handle AD points!
  return _dilatation_function.value(MetaPhysicL::raw_value(temperature), Point());
}
