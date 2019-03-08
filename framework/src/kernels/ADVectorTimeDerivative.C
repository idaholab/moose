//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorTimeDerivative.h"

registerADMooseObject("MooseApp", ADVectorTimeDerivative);

defineADValidParams(
    ADVectorTimeDerivative,
    ADVectorTimeKernel,
    params.addClassDescription("The time derivative operator with the weak form of $(\\psi_i, "
                               "\\frac{\\partial u_h}{\\partial t})$."););

template <ComputeStage compute_stage>
ADVectorTimeDerivative<compute_stage>::ADVectorTimeDerivative(const InputParameters & parameters)
  : ADVectorTimeKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
ADVectorTimeDerivative<compute_stage>::precomputeQpResidual()
{
  return _u_dot[_qp];
}

adBaseClass(ADVectorTimeDerivative);
