//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RobinBC.h"

template <>
InputParameters
validParams<RobinBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

RobinBC::RobinBC(const InputParameters & parameters) : IntegratedBC(parameters) {}

Real
RobinBC::computeQpResidual()
{
  return _test[_i][_qp] * 2. * _u[_qp];
  // return _test[_i][_qp] * -2. * _grad_u[_qp] * _normals[_qp];
}

Real
RobinBC::computeQpJacobian()
{
  return _test[_i][_qp] * 2. * _phi[_j][_qp];
  // return _test[_i][_qp] * -2. * _grad_phi[_j][_qp] * _normals[_qp];
}
