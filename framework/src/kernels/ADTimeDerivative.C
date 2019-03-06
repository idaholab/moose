//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTimeDerivative.h"

registerADMooseObject("MooseApp", ADTimeDerivative);

defineADValidParams(
    ADTimeDerivative,
    ADTimeKernel,
    params.addClassDescription("The time derivative operator with the weak form of $(\\psi_i, "
                               "\\frac{\\partial u_h}{\\partial t})$."););

template <ComputeStage compute_stage>
ADTimeDerivative<compute_stage>::ADTimeDerivative(const InputParameters & parameters)
  : ADTimeKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADResidual
ADTimeDerivative<compute_stage>::precomputeQpResidual()
{
  return _u_dot[_qp];
}

adBaseClass(ADTimeDerivative);
