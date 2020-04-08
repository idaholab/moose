//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDiffusion.h"

registerADMooseObject("MooseApp", ADDiffusion);

defineADLegacyParams(ADDiffusion);

template <ComputeStage compute_stage>
InputParameters
ADDiffusion<compute_stage>::validParams()
{
  auto params = ADKernelGrad<compute_stage>::validParams();
  params.addClassDescription("Same as `Diffusion` in terms of physics/residual, but the Jacobian "
                             "is computed using forward automatic differentiation");
  return params;
}

template <ComputeStage compute_stage>
ADDiffusion<compute_stage>::ADDiffusion(const InputParameters & parameters)
  : ADKernelGrad<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
ADDiffusion<compute_stage>::precomputeQpResidual()
{
  return _grad_u[_qp];
}

adBaseClass(ADDiffusion);
