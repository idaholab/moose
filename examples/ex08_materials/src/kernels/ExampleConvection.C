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

template <>
InputParameters
validParams<ExampleConvection>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

ExampleConvection::ExampleConvection(const InputParameters & parameters)
  : Kernel(parameters),

    // Retrieve a gradient material property to use for the convection
    // velocity
    _velocity(getMaterialProperty<RealGradient>("convection_velocity"))
{
}

Real
ExampleConvection::computeQpResidual()
{
  return _test[_i][_qp] * (_velocity[_qp] * _grad_u[_qp]);
}

Real
ExampleConvection::computeQpJacobian()
{
  return _test[_i][_qp] * (_velocity[_qp] * _grad_phi[_j][_qp]);
}
