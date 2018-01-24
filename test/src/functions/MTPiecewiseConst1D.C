//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTPiecewiseConst1D.h"

template <>
InputParameters
validParams<MTPiecewiseConst1D>()
{
  InputParameters params = validParams<Function>();
  return params;
}

MTPiecewiseConst1D::MTPiecewiseConst1D(const InputParameters & parameters) : Function(parameters) {}

Real
MTPiecewiseConst1D::value(Real /*t*/, const Point & p)
{
  Real val = 0;
  Real x = p(0);

  if ((x >= -0.75 && x < -0.50) || (x >= -0.25 && x < 0.25) || (x >= 0.50 && x < 0.75))
    val = 1.0;

  else
    val = 0.1;

  return val;
}
