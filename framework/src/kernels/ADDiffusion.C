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

defineADValidParams(
    ADDiffusion,
    ADKernel,
    params.addClassDescription("Same as `Diffusion` in terms of physics/residual, but the Jacobian "
                               "is computed using forward automatic differentiation"););

template <ComputeStage compute_stage>
ADDiffusion<compute_stage>::ADDiffusion(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADResidual
ADDiffusion<compute_stage>::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}

template class ADDiffusion<RESIDUAL>;
template class ADDiffusion<JACOBIAN>;
