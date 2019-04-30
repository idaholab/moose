//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPresetBC.h"

registerADMooseObject("MooseApp", ADPresetBC);

defineADValidParams(
    ADPresetBC, ADPresetNodalBC, params.addRequiredParam<Real>("value", "Value of the BC");
    params.declareControllable("value");
    params.addClassDescription(
        "Similar to DirichletBC except the value is applied before the solve begins"););

template <ComputeStage compute_stage>
ADPresetBC<compute_stage>::ADPresetBC(const InputParameters & parameters)
  : ADPresetNodalBC<compute_stage>(parameters), _value(adGetParam<Real>("value"))
{
}

template <ComputeStage compute_stage>
ADReal
ADPresetBC<compute_stage>::computeQpValue()
{
  return _value;
}
