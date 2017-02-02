/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetAdvectionSUPG.h"

template<>
InputParameters validParams<LevelSetAdvectionSUPG>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("SUPG stablization term for the advection portion of the level set equation.");
  params += validParams<LevelSetVelocityInterface<> >();
  return params;
}

LevelSetAdvectionSUPG::LevelSetAdvectionSUPG(const InputParameters & parameters) :
    LevelSetVelocityInterface<Kernel>(parameters)
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
