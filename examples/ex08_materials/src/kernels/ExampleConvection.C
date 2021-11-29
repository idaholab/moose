//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleConvection.h"

registerMooseObject("ExampleApp", ExampleConvection);

InputParameters
ExampleConvection::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

ExampleConvection::ExampleConvection(const InputParameters & parameters)
  : Kernel(parameters),
    // Retrieve and store gradient from a material property to use for the convection velocity
    _velocity(getMaterialProperty<RealGradient>("convection_velocity"))
{
}

Real
ExampleConvection::computeQpResidual()
{
  // Use the velocity gradient just like before in example 3
  return _test[_i][_qp] * (_velocity[_qp] * _grad_u[_qp]);
}

Real
ExampleConvection::computeQpJacobian()
{
  return _test[_i][_qp] * (_velocity[_qp] * _grad_phi[_j][_qp]);
}
