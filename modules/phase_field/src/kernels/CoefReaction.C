/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoefReaction.h"

template <>
InputParameters
validParams<CoefReaction>()
{
  InputParameters params = validParams<Reaction>();
  params.addClassDescription("Implements the residual term (p*u, test)");
  params.addRequiredParam<Real>("coefficient", "Coefficient of the term");
  return params;
}

CoefReaction::CoefReaction(const InputParameters & parameters)
  : Reaction(parameters), _coef(getParam<Real>("coefficient"))
{
}

Real
CoefReaction::computeQpResidual()
{
  return _coef * Reaction::computeQpResidual();
}

Real
CoefReaction::computeQpJacobian()
{
  return _coef * Reaction::computeQpJacobian();
}
