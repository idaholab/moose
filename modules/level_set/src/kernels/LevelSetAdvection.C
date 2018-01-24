//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LevelSetAdvection.h"

template <>
InputParameters
validParams<LevelSetAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Implements the level set advection equation: $\\vec{v}\\cdot\\nabla "
                             "u = 0$, where the weak form is $(\\psi_i, \\vec{v}\\cdot\\nabla u) = "
                             "0$.");
  params += validParams<LevelSetVelocityInterface<>>();
  return params;
}

LevelSetAdvection::LevelSetAdvection(const InputParameters & parameters)
  : LevelSetVelocityInterface<Kernel>(parameters)
{
}

Real
LevelSetAdvection::computeQpResidual()
{
  computeQpVelocity();
  return _test[_i][_qp] * (_velocity * _grad_u[_qp]);
}

Real
LevelSetAdvection::computeQpJacobian()
{
  computeQpVelocity();
  return _test[_i][_qp] * (_velocity * _grad_phi[_j][_qp]);
}
