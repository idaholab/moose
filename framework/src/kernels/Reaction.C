//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Reaction.h"

template <>
InputParameters
validParams<Reaction>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Implements a simple consuming reaction term with weak form $(\\psi_i, u_h)$.");
  return params;
}

Reaction::Reaction(const InputParameters & parameters) : Kernel(parameters) {}

Real
Reaction::computeQpResidual()
{
  return _test[_i][_qp] * _u[_qp];
}

Real
Reaction::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp];
}
