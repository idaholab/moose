//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionPrecompute.h"

registerADMooseObject("MooseTestApp", ADConvectionPrecompute);

/**
 * This macro defines the valid parameters for
 * this Kernel and their default values
 */
defineADValidParams(ADConvectionPrecompute,
                    ADKernelValue,
                    params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector"););

template <ComputeStage compute_stage>
ADConvectionPrecompute<compute_stage>::ADConvectionPrecompute(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

template <ComputeStage compute_stage>
ADReal
ADConvectionPrecompute<compute_stage>::precomputeQpResidual()
{
  // velocity * _grad_u[_qp] is actually doing a dot product
  return _velocity * _grad_u[_qp];
}
