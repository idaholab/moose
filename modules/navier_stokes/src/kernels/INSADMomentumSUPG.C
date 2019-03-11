//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumSUPG.h"

registerADMooseObject("NavierStokesApp", INSADMomentumSUPG);

defineADValidParams(
    INSADMomentumSUPG,
    ADVectorKernelSUPG,
    params.addClassDescription("Adds the supg stabilization to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumSUPG<compute_stage>::INSADMomentumSUPG(const InputParameters & parameters)
  : ADVectorKernelSUPG<compute_stage>(parameters),
    _momentum_strong_residual(adGetADMaterialProperty<RealVectorValue>("momentum_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
INSADMomentumSUPG<compute_stage>::precomputeQpStrongResidual()
{
  return _momentum_strong_residual[_qp];
}
