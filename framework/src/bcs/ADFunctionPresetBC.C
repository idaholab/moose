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

registerADMooseObjectDeprecated("MooseApp", ADFunctionPresetBC, "06/30/2020 24:00");

defineADLegacyParams(ADFunctionPresetBC);

template <ComputeStage compute_stage>
InputParameters
ADFunctionPresetBC<compute_stage>::validParams()
{
  InputParameters params = ADDirichletBCBase<compute_stage>::validParams();

  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "The same as ADFunctionDirichletBC except the value is applied before the solve begins");

  // Utilize the new ADDirichletBCBase with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

template <ComputeStage compute_stage>
ADFunctionPresetBC<compute_stage>::ADFunctionPresetBC(const InputParameters & parameters)
  : ADDirichletBCBase<compute_stage>(parameters), _func(getFunction("function"))
{
  mooseDeprecated("Use ADFunctionDirichletBC with preset = true instead of ADFunctionPresetBC");
}

template <ComputeStage compute_stage>
ADReal
ADFunctionPresetBC<compute_stage>::computeQpValue()
{
  return _func.value(_t, *_current_node);
}
