//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetForcingFunctionSUPG.h"
#include "Function.h"

registerADMooseObject("LevelSetApp", LevelSetForcingFunctionSUPG);

defineADValidParams(
    LevelSetForcingFunctionSUPG,
    ADKernelGrad,
    params.addClassDescription("The SUPG stablization term for a forcing function.");
    params.addParam<FunctionName>("function", "1", "A function that describes the body force");
    params += validParams<LevelSetVelocityInterface<>>(););

template <ComputeStage compute_stage>
LevelSetForcingFunctionSUPG<compute_stage>::LevelSetForcingFunctionSUPG(
    const InputParameters & parameters)
  : LevelSetVelocityInterface<ADKernelGrad<compute_stage>>(parameters),
    _function(getFunction("function"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
LevelSetForcingFunctionSUPG<compute_stage>::precomputeQpResidual()
{
  computeQpVelocity();
  ADReal tau = _current_elem->hmin() / (2 * _velocity.norm());
  return -tau * _velocity * _function.value(_t, _q_point[_qp]);
}
