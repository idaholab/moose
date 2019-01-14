//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDiffusionPrecompute.h"

registerADMooseObject("MooseTestApp", ADDiffusionPrecompute);

defineADValidParams(ADDiffusionPrecompute, ADKernelGrad, );

template <ComputeStage compute_stage>
ADDiffusionPrecompute<compute_stage>::ADDiffusionPrecompute(const InputParameters & parameters)
  : ADKernelGrad<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADGradResidual
ADDiffusionPrecompute<compute_stage>::precomputeQpResidual()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_u[_qp];
}
