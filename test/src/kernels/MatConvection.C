//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatConvection.h"

registerMooseObject("MooseTestApp", MatConvection);

InputParameters
MatConvection::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop", "Name of the property (scalar) to multiply the MatConvection kernel with");

  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

MatConvection::MatConvection(const InputParameters & parameters)
  : Kernel(parameters),
    _conv_prop(getMaterialProperty<Real>("mat_prop")),
    _x(getParam<Real>("x")),
    _y(getParam<Real>("y")),
    _z(getParam<Real>("z"))
{
  _velocity(0) = _x;
  _velocity(1) = _y;
  _velocity(2) = _z;
}

Real
MatConvection::computeQpResidual()
{
  return _test[_i][_qp] * (_conv_prop[_qp] * _velocity * _grad_u[_qp]);
}

Real
MatConvection::computeQpJacobian()
{
  return _test[_i][_qp] * (_conv_prop[_qp] * _velocity * _grad_phi[_j][_qp]);
}
