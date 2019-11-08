//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Advection0.h"

registerMooseObject("MooseTestApp", Advection0);

InputParameters
Advection0::validParams()
{
  InputParameters params = Kernel::validParams();

  params.set<Real>("Au") = 1.0;
  params.set<Real>("Bu") = 1.0;
  params.set<Real>("Cu") = 1.0;

  params.set<Real>("Av") = 1.0;
  params.set<Real>("Bv") = 1.0;
  params.set<Real>("Cv") = 1.0;

  return params;
}

Advection0::Advection0(const InputParameters & parameters) : Kernel(parameters)
{
  _Au = getParam<Real>("Au");
  _Bu = getParam<Real>("Bu");
  _Cu = getParam<Real>("Cu");

  _Av = getParam<Real>("Av");
  _Bv = getParam<Real>("Bv");
  _Cv = getParam<Real>("Cv");
}

Real
Advection0::computeQpResidual()
{
  VectorValue<Number> vel(_Au + _Bu * _q_point[_qp](0) + _Cu * _q_point[_qp](1),
                          _Av + _Bv * _q_point[_qp](0) + _Cv * _q_point[_qp](1),
                          0.0);
  return -_test[_i][_qp] * vel * _grad_u[_qp];
}

Real
Advection0::computeQpJacobian()
{
  VectorValue<Number> vel(_Au + _Bu * _q_point[_qp](0) + _Cu * _q_point[_qp](1),
                          _Av + _Bv * _q_point[_qp](0) + _Cv * _q_point[_qp](1),
                          0.0);
  return -_test[_i][_qp] * vel * _grad_phi[_j][_qp];
}
