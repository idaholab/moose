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
    ADKernelValue,
    params.addClassDescription("Adds the convective term to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumAdvection<compute_stage>::INSADMomentumAdvection(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters),
    _convective_strong_residual(
        adGetADMaterialProperty<RealVectorValue>("convective_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
INSADMomentumAdvection<compute_stage>::precomputeQpResidual()
{
  return _convective_strong_residual;
}

registerADMooseObject("NavierStokesApp", INSADMomentumAdvectionSUPG);

defineADValidParams(
    INSADMomentumAdvectionSUPG,
    ADKernelGrad,
    params.addClassDescription("Adds the supg convective term to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumAdvectionSUPG<compute_stage>::INSADMomentumAdvectionSUPG(
    const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters),
    _convective_strong_residual(
        adGetADMaterialProperty<RealVectorValue>("convective_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
INSADMomentumAdvectionSUPG<compute_stage>::computeQpResidual()
{
  return _convective_strong_residual;
}
