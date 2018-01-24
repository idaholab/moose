//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetTimeDerivativeSUPG.h"

template <>
InputParameters
validParams<LevelSetTimeDerivativeSUPG>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription(
      "SUPG stablization terms for the time derivative of the level set equation.");
  params += validParams<LevelSetVelocityInterface<>>();
  return params;
}

LevelSetTimeDerivativeSUPG::LevelSetTimeDerivativeSUPG(const InputParameters & parameters)
  : LevelSetVelocityInterface<TimeDerivative>(parameters)
{
}

Real
LevelSetTimeDerivativeSUPG::computeQpResidual()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return tau * _velocity * _grad_test[_i][_qp] * _u_dot[_qp];
}

Real
LevelSetTimeDerivativeSUPG::computeQpJacobian()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return tau * _velocity * _grad_test[_i][_qp] * _phi[_j][_qp] * _du_dot_du[_qp];
}
