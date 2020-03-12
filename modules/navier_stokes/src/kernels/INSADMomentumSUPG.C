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

defineADLegacyParams(INSADMomentumSUPG);

template <ComputeStage compute_stage>
InputParameters
INSADMomentumSUPG<compute_stage>::validParams()
{
  InputParameters params = ADVectorKernelSUPG<compute_stage>::validParams();
  params.addClassDescription("Adds the supg stabilization to the INS momentum equation");
  return params;
}

template <ComputeStage compute_stage>
INSADMomentumSUPG<compute_stage>::INSADMomentumSUPG(const InputParameters & parameters)
  : ADVectorKernelSUPG<compute_stage>(parameters),
    _momentum_strong_residual(getADMaterialProperty<RealVectorValue>("momentum_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
INSADMomentumSUPG<compute_stage>::precomputeQpStrongResidual()
{
  return _momentum_strong_residual[_qp];
}
