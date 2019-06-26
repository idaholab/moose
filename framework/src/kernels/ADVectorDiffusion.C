//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorDiffusion.h"

registerADMooseObject("MooseApp", ADVectorDiffusion);

defineADValidParams(
    ADVectorDiffusion,
    ADVectorKernel,
    params.addClassDescription(
        "The Laplacian operator ($-\\nabla \\cdot \\nabla \\vec{u}$), with the weak "
        "form of $(\\nabla \\vec{\\phi_i}, \\nabla \\vec{u_h})$. The Jacobian is computed using "
        "automatic differentiation"););

template <ComputeStage compute_stage>
ADVectorDiffusion<compute_stage>::ADVectorDiffusion(const InputParameters & parameters)
  : ADVectorKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADReal
ADVectorDiffusion<compute_stage>::computeQpResidual()
{
  return _grad_u[_qp].contract(_grad_test[_i][_qp]);
}

template class ADVectorDiffusion<RESIDUAL>;
template class ADVectorDiffusion<JACOBIAN>;
