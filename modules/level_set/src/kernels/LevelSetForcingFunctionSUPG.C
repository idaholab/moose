/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
