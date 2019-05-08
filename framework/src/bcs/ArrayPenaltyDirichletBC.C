//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayPenaltyDirichletBC.h"

registerMooseObject("MooseApp", ArrayPenaltyDirichletBC);

template <>
InputParameters
validParams<ArrayPenaltyDirichletBC>()
{
  InputParameters params = validParams<ArrayIntegratedBC>();
  params.addParam<Real>("penalty", 4, "Penalty scalar");
  params.addRequiredParam<RealArrayValue>("value", "Boundary value of the array variable");
  return params;
}

ArrayPenaltyDirichletBC::ArrayPenaltyDirichletBC(const InputParameters & parameters)
  : ArrayIntegratedBC(parameters),
    _p(getParam<Real>("penalty")),
    _v(getParam<RealArrayValue>("value"))
{
}

RealArrayValue
ArrayPenaltyDirichletBC::computeQpResidual()
{
  return _p * _test[_i][_qp] * (_u[_qp] - _v);
}

RealArrayValue
ArrayPenaltyDirichletBC::computeQpJacobian()
{
  return RealArrayValue::Constant(_count, _p * _phi[_j][_qp] * _test[_i][_qp]);
}
