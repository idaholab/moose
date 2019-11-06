//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectionPrecompute.h"

registerMooseObject("MooseTestApp", ConvectionPrecompute);

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
InputParameters
ConvectionPrecompute::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  return params;
}

ConvectionPrecompute::ConvectionPrecompute(const InputParameters & parameters)
  : KernelValue(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
ConvectionPrecompute::precomputeQpResidual()
{
  // velocity * _grad_u[_qp] is actually doing a dot product
  return _velocity * _grad_u[_qp];
}

Real
ConvectionPrecompute::precomputeQpJacobian()
{
  // the partial derivative of _grad_u is just _grad_phi[_j]
  return _velocity * _grad_phi[_j][_qp];
}
