//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TEJumpBC.h"

registerMooseObject("MooseTestApp", TEJumpBC);

InputParameters
TEJumpBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEJumpBC::TEJumpBC(const InputParameters & parameters)
  : NodalBC(parameters), _t_jump(getParam<Real>("t_jump")), _slope(getParam<Real>("slope"))
{
}

Real
TEJumpBC::computeQpResidual()
{
  return _u[_qp] - (atan((_t - _t_jump) * libMesh::pi * _slope));
}
