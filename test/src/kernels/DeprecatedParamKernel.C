//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeprecatedParamKernel.h"

InputParameters
DeprecatedParamKernel::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  return params;
}

DeprecatedParamKernel::DeprecatedParamKernel(const InputParameters & parameters)
  : Reaction(parameters), _coef(getParam<Real>("coefficient"))
{
}

Real
DeprecatedParamKernel::computeQpResidual()
{
  return _coef * Reaction::computeQpResidual();
}

Real
DeprecatedParamKernel::computeQpJacobian()
{
  return _coef * Reaction::computeQpJacobian();
}
