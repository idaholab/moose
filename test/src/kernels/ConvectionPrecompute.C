/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "ConvectionPrecompute.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template <>
InputParameters
validParams<ConvectionPrecompute>()
{
  InputParameters params = validParams<KernelValue>();
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
