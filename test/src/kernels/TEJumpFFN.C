//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TEJumpFFN.h"

registerMooseObject("MooseTestApp", TEJumpFFN);

InputParameters
TEJumpFFN::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEJumpFFN::TEJumpFFN(const InputParameters & parameters)
  : Kernel(parameters), _t_jump(getParam<Real>("t_jump")), _slope(getParam<Real>("slope"))
{
}

Real
TEJumpFFN::computeQpResidual()
{
  return -_test[_i][_qp] * (_slope * libMesh::pi) /
         (1 + _slope * _slope * libMesh::pi * libMesh::pi * std::pow(_t_jump - _t, 2));
}

Real
TEJumpFFN::computeQpJacobian()
{
  return 0;
}
