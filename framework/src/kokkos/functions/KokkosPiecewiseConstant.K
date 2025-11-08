//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPiecewiseConstant.h"

registerKokkosFunction("MooseApp", KokkosPiecewiseConstant);

InputParameters
KokkosPiecewiseConstant::validParams()
{
  InputParameters params = KokkosPiecewiseTabularBase::validParams();
  MooseEnum direction("LEFT RIGHT LEFT_INCLUSIVE RIGHT_INCLUSIVE", "LEFT");
  params.addParam<MooseEnum>(
      "direction", direction, "Direction to look to find value: " + direction.getRawNames());
  params.addClassDescription("Defines piece-wise constant data using a set of x-y data pairs");
  return params;
}

KokkosPiecewiseConstant::KokkosPiecewiseConstant(const InputParameters & parameters)
  : KokkosPiecewiseTabularBase(parameters),
    _direction(getParam<MooseEnum>("direction").getEnum<Direction>())
{
}
