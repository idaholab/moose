//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PReaction.h"

registerMooseObject("MooseTestApp", PReaction);

InputParameters
PReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  params.addParam<Real>("power", 1.0, "Variable power");
  return params;
}

PReaction::PReaction(const InputParameters & parameters)
  : Kernel(parameters), _coef(getParam<Real>("coefficient")), _p(getParam<Real>("power"))
{
}

Real
PReaction::computeQpResidual()
{
  return _coef * _test[_i][_qp] * std::pow(_u[_qp], _p);
}

Real
PReaction::computeQpJacobian()
{
  return _coef * _p * std::pow(_u[_qp], _p - 1) * _test[_i][_qp] * _phi[_j][_qp];
}
