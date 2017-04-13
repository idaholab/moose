/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "CoefReaction.h"

template <>
InputParameters
validParams<CoefReaction>()
{
  InputParameters params = validParams<Reaction>();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
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
