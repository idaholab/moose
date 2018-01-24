//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradientJumpIndicator.h"

template <>
InputParameters
validParams<GradientJumpIndicator>()
{
  InputParameters params = validParams<InternalSideIndicator>();
  return params;
}

GradientJumpIndicator::GradientJumpIndicator(const InputParameters & parameters)
  : InternalSideIndicator(parameters)
{
}

Real
GradientJumpIndicator::computeQpIntegral()
{
  Real jump = (_grad_u[_qp] - _grad_u_neighbor[_qp]) * _normals[_qp];

  return jump * jump;
}
