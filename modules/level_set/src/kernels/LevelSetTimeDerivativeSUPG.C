//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetTimeDerivativeSUPG.h"

registerADMooseObject("LevelSetApp", LevelSetTimeDerivativeSUPG);

defineADValidParams(
    LevelSetTimeDerivativeSUPG,
    ADTimeKernelGrad,
    params.addClassDescription(
        "SUPG stablization terms for the time derivative of the level set equation.");
    params += validParams<LevelSetVelocityInterface<>>(););

template <ComputeStage compute_stage>
LevelSetTimeDerivativeSUPG<compute_stage>::LevelSetTimeDerivativeSUPG(
    const InputParameters & parameters)
  : LevelSetVelocityInterface<ADTimeKernelGrad<compute_stage>>(parameters)
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
LevelSetTimeDerivativeSUPG<compute_stage>::precomputeQpResidual()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return tau * _velocity * _u_dot[_qp];
}
