//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RobinBC.h"

registerMooseObject("MooseTestApp", RobinBC);

InputParameters
RobinBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<Real>("coef", 2, "The cofficient multiplying this BC's residual/Jacobian");
  return params;
}

RobinBC::RobinBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _coef(getParam<Real>("coef"))
{
}

Real
RobinBC::computeQpResidual()
{
  return _test[_i][_qp] * _coef * _u[_qp];
}

Real
RobinBC::computeQpJacobian()
{
  return _test[_i][_qp] * _coef * _phi[_j][_qp];
}
