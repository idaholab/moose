//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpiredKernel.h"

registerMooseObjectDeprecated("MooseTestApp", ExpiredKernel, "01/01/2018 00:00");

InputParameters
ExpiredKernel::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  return params;
}

ExpiredKernel::ExpiredKernel(const InputParameters & parameters)
  : Reaction(parameters), _coef(getParam<Real>("coefficient"))
{
}

Real
ExpiredKernel::computeQpResidual()
{
  return _coef * Reaction::computeQpResidual();
}

Real
ExpiredKernel::computeQpJacobian()
{
  return _coef * Reaction::computeQpJacobian();
}
