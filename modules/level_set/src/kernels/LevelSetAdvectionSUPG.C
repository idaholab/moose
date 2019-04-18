//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetAdvectionSUPG.h"

registerADMooseObject("LevelSetApp", LevelSetAdvectionSUPG);

defineADValidParams(
    LevelSetAdvectionSUPG,
    ADKernelGrad,
    params.addClassDescription(
        "SUPG stablization term for the advection portion of the level set equation.");
    params += validParams<LevelSetVelocityInterface<>>(););

template <ComputeStage compute_stage>
LevelSetAdvectionSUPG<compute_stage>::LevelSetAdvectionSUPG(const InputParameters & parameters)
  : LevelSetVelocityInterface<ADKernelGrad<compute_stage>>(parameters)
{
}

template <ComputeStage compute_stage>
ADVectorResidual
LevelSetAdvectionSUPG<compute_stage>::precomputeQpResidual()
{
  computeQpVelocity();
  ADReal tau = _current_elem->hmin() / (2 * _velocity.norm());
  return (tau * _velocity) * (_velocity * _grad_u[_qp]);
}
