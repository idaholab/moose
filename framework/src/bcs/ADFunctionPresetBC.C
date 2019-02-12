//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctionPresetBC.h"
#include "Function.h"

registerADMooseObject("MooseApp", ADFunctionPresetBC);

defineADValidParams(
    ADFunctionPresetBC,
    ADPresetNodalBC,
    params.addRequiredParam<FunctionName>("function", "The forcing function.");
    params.addClassDescription(
        "The same as FunctionDirichletBC except the value is applied before the solve begins"););

template <ComputeStage compute_stage>
ADFunctionPresetBC<compute_stage>::ADFunctionPresetBC(const InputParameters & parameters)
  : ADPresetNodalBC<compute_stage>(parameters), _func(getFunction("function"))
{
}

template <ComputeStage compute_stage>
ADReal
ADFunctionPresetBC<compute_stage>::computeQpValue()
{
  return _func.value(_t, *_current_node);
}
