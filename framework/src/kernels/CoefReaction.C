//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefReaction.h"

registerMooseObject("MooseApp", CoefReaction);

InputParameters
CoefReaction::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addClassDescription("Implements the residual term (p*u, test)");
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
