//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Reaction.h"

registerMooseObject("MooseApp", Reaction);

defineLegacyParams(Reaction);

InputParameters
Reaction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term with weak form $(\\psi_i, \\lambda u_h)$.");
  params.addParam<Real>(
      "rate", 1.0, "The $(\\lambda)$ multiplier, the relative amount consumed per unit time.");
  return params;
}

Reaction::Reaction(const InputParameters & parameters)
  : Kernel(parameters), _rate(getParam<Real>("rate"))
{
}

Real
Reaction::computeQpResidual()
{
  return _test[_i][_qp] * _rate * _u[_qp];
}

Real
Reaction::computeQpJacobian()
{
  return _test[_i][_qp] * _rate * _phi[_j][_qp];
}
