//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ValueJumpIndicator.h"

template <>
InputParameters
validParams<ValueJumpIndicator>()
{
  InputParameters params = validParams<InternalSideIndicator>();
  return params;
}

ValueJumpIndicator::ValueJumpIndicator(const InputParameters & parameters)
  : InternalSideIndicator(parameters)
{
}

Real
ValueJumpIndicator::computeQpIntegral()
{
  Real jump = _u[_qp] - _u_neighbor[_qp];

  return jump * jump;
}
