//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumAdvection.h"

registerADMooseObject("NavierStokesApp", INSADMomentumAdvection);

defineADValidParams(
    INSADMomentumAdvection,
    ADVectorKernelValue,
    params.addClassDescription("Adds the convective term to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumAdvection<compute_stage>::INSADMomentumAdvection(const InputParameters & parameters)
  : ADVectorKernelValue<compute_stage>(parameters),
    _convective_strong_residual(
        adGetADMaterialProperty<RealVectorValue>("convective_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
INSADMomentumAdvection<compute_stage>::precomputeQpResidual()
{
  return _convective_strong_residual[_qp];
}
