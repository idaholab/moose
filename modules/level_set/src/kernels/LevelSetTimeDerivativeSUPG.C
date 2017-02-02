/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetTimeDerivativeSUPG.h"

template<>
InputParameters validParams<LevelSetTimeDerivativeSUPG>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription("SUPG stablization terms for the time derivative of the level set equation.");
  params += validParams<LevelSetVelocityInterface<> >();
  return params;
}

LevelSetTimeDerivativeSUPG::LevelSetTimeDerivativeSUPG(const InputParameters & parameters) :
    LevelSetVelocityInterface<TimeDerivative>(parameters)
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
  return tau * _velocity * _grad_test[_i][_qp] * _phi[_j][_qp]*_du_dot_du[_qp];
}
