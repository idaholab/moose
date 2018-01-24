//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetAdvectionSUPG.h"

template <>
InputParameters
validParams<LevelSetAdvectionSUPG>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "SUPG stablization term for the advection portion of the level set equation.");
  params += validParams<LevelSetVelocityInterface<>>();
  return params;
}

LevelSetAdvectionSUPG::LevelSetAdvectionSUPG(const InputParameters & parameters)
  : LevelSetVelocityInterface<Kernel>(parameters)
{
}

Real
LevelSetAdvectionSUPG::computeQpResidual()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return (tau * _velocity * _grad_test[_i][_qp]) * (_velocity * _grad_u[_qp]);
}

Real
LevelSetAdvectionSUPG::computeQpJacobian()
{
  computeQpVelocity();
  Real tau = _current_elem->hmin() / (2 * _velocity.norm());
  return (tau * _velocity * _grad_test[_i][_qp]) * (_velocity * _grad_phi[_j][_qp]);
}
