//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGFunctionConvectionDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseTestApp", DGFunctionConvectionDirichletBC);

InputParameters
DGFunctionConvectionDirichletBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addRequiredParam<Real>("x", "Component of the velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of the velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of the velocity in the z direction");
  return params;
}

DGFunctionConvectionDirichletBC::DGFunctionConvectionDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _func(getFunction("function")),
    _x(getParam<Real>("x")),
    _y(getParam<Real>("y")),
    _z(getParam<Real>("z"))
{
  _velocity(0) = _x;
  _velocity(1) = _y;
  _velocity(2) = _z;
}

Real
DGFunctionConvectionDirichletBC::computeQpResidual()
{
  Real fn = _func.value(_t, _q_point[_qp]);
  Real r = 0;
  if (_velocity * _normals[_qp] >= 0)
  {
    r += (_velocity * _normals[_qp]) * (_u[_qp] - fn) * _test[_i][_qp];
  }
  else
  {
    r += (_velocity * _normals[_qp]) * (fn - _u[_qp]) * _test[_i][_qp];
  }

  return r;
}

Real
DGFunctionConvectionDirichletBC::computeQpJacobian()
{
  Real r = 0;
  if (_velocity * _normals[_qp] >= 0)
  {
    r += (_velocity * _normals[_qp]) * _test[_j][_qp] * _test[_i][_qp];
  }
  else
  {
    r -= (_velocity * _normals[_qp]) * _test[_j][_qp] * _test[_i][_qp];
  }

  return r;
}
