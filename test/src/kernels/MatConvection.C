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
#include "MatConvection.h"

template <>
InputParameters
validParams<MatConvection>()
{
  InputParameters params = validParams<Kernel>();
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
