//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionPrecompute.h"

registerMooseObject("MooseTestApp", ADConvectionPrecompute);

/**
 * This macro defines the valid parameters for
 * this Kernel and their default values
 */
InputParameters
ADConvectionPrecompute::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  return params;
}

ADConvectionPrecompute::ADConvectionPrecompute(const InputParameters & parameters)
  : ADKernelValue(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

ADReal
ADConvectionPrecompute::precomputeQpResidual()
{
  // velocity * _grad_u[_qp] is actually doing a dot product
  return _velocity * _grad_u[_qp];
}
