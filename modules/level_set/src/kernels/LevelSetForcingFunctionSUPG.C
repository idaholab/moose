//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetForcingFunctionSUPG.h"
#include "Function.h"

template <>
InputParameters
validParams<LevelSetForcingFunctionSUPG>()
{
  InputParameters params = validParams<BodyForce>();
  params.addClassDescription("The SUPG stablization term for a forcing function.");
  params += validParams<LevelSetVelocityInterface<>>();
  return params;
}

LevelSetForcingFunctionSUPG::LevelSetForcingFunctionSUPG(const InputParameters & parameters)
  : LevelSetVelocityInterface<BodyForce>(parameters)
{
}

Real
LevelSetForcingFunctionSUPG::computeQpResidual()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return -tau * _velocity * _grad_test[_i][_qp] * _function.value(_t, _q_point[_qp]);
}

Real
LevelSetForcingFunctionSUPG::computeQpJacobian()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return -tau * _velocity * _grad_test[_i][_qp] * _function.value(_t, _q_point[_qp]);
}
