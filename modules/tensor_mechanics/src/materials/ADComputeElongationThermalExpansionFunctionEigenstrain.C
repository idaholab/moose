//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeElongationThermalExpansionFunctionEigenstrain.h"

registerADMooseObject("TensorMechanicsApp", ADComputeElongationThermalExpansionFunctionEigenstrain);

defineADLegacyParams(ADComputeElongationThermalExpansionFunctionEigenstrain);

template <ComputeStage compute_stage>
InputParameters
ADComputeElongationThermalExpansionFunctionEigenstrain<compute_stage>::validParams()
{
  InputParameters params =
      ADComputeElongationThermalExpansionEigenstrainBase<compute_stage>::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the total elongation as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "elongation_function",
      "Function describing the thermal elongation as a function of temperature");
  return params;
}

template <ComputeStage compute_stage>
ADComputeElongationThermalExpansionFunctionEigenstrain<compute_stage>::
    ADComputeElongationThermalExpansionFunctionEigenstrain(const InputParameters & parameters)
  : ADComputeElongationThermalExpansionEigenstrainBase<compute_stage>(parameters),
    _elongation_function(getFunction("elongation_function"))
{
}

template <ComputeStage compute_stage>
ADReal
ADComputeElongationThermalExpansionFunctionEigenstrain<compute_stage>::computeElongation(
    const ADReal & temperature)
{
  // Note: All dual number information will be lost until functions can handle AD points!
  return _elongation_function.value(MetaPhysicL::raw_value(temperature), Point());
}
