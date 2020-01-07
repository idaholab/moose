//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Diffusion0.h"

registerMooseObject("MooseTestApp", Diffusion0);

InputParameters
Diffusion0::validParams()
{
  InputParameters params = Kernel::validParams();

  params.set<Real>("Ak") = 1.0;
  params.set<Real>("Bk") = 1.0;
  params.set<Real>("Ck") = 1.0;

  return params;
}

Diffusion0::Diffusion0(const InputParameters & parameters) : Kernel(parameters)
{
  _Ak = getParam<Real>("Ak");
  _Bk = getParam<Real>("Bk");
  _Ck = getParam<Real>("Ck");
}

Real
Diffusion0::computeQpResidual()
{
  Real diff = _Ak + _Bk * _q_point[_qp](0) + _Ck * _q_point[_qp](1);

  return diff * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
Diffusion0::computeQpJacobian()
{
  Real diff = _Ak + _Bk * _q_point[_qp](0) + _Ck * _q_point[_qp](1);

  return diff * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
