//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleConvection.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
registerMooseObject("ExampleApp", ExampleConvection);

template <>
InputParameters
validParams<ExampleConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

ExampleConvection::ExampleConvection(const InputParameters & parameters)
  : // You must call the constructor of the base class first
    // The "true" here specifies that this Kernel is to be integrated
    // over the domain.
    Kernel(parameters),

    // This is the "initialization List" it sets the values of class variables
    // Here we are grabbing the values of Parameters to use for a velocity vector
    _x(getParam<Real>("x")),
    _y(getParam<Real>("y")),
    _z(getParam<Real>("z"))
{
  // Build a velocity vector to use in the residual / jacobian computations.
  // We do this here so that it's only done once and then we just reuse it.
  // Note that RealVectorValues ALWAYS have 3 components... even when running in
  // 2D or 1D.  This makes the code simpler...
  _velocity(0) = _x;
  _velocity(1) = _y;
  _velocity(2) = _z;
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
