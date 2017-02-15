/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "LevelSetAdvection.h"

template<>
InputParameters validParams<LevelSetAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Implements the level set advection equation: $\\vec{v}\\cdot\\nabla u = 0$, where the weak form is $(\\psi_i, \\vec{v}\\cdot\\nabla u) = 0$.");
  params += validParams<LevelSetVelocityInterface<> >();
  return params;
}

LevelSetAdvection::LevelSetAdvection(const InputParameters & parameters) :
    LevelSetVelocityInterface<Kernel>(parameters)
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
