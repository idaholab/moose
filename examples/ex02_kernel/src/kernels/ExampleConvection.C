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

#include "ExampleConvection.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template <>
InputParameters
validParams<ExampleConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  return params;
}

ExampleConvection::ExampleConvection(const InputParameters & parameters)
  : // You must call the constructor of the base class first
    Kernel(parameters),
    _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
ExampleConvection::computeQpResidual()
{
  // velocity * _grad_u[_qp] is actually doing a dot product
  return _test[_i][_qp] * (_velocity * _grad_u[_qp]);
}

Real
ExampleConvection::computeQpJacobian()
{
  // the partial derivative of _grad_u is just _grad_phi[_j]
  return _test[_i][_qp] * (_velocity * _grad_phi[_j][_qp]);
}
