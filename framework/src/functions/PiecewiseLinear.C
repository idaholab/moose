//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseLinear.h"

registerMooseObject("MooseApp", PiecewiseLinear);

defineLegacyParams(PiecewiseLinear);

InputParameters
PiecewiseLinear::validParams()
{
  InputParameters params = PiecewiseLinearBase::validParams();
  params.addClassDescription("Linearly interpolates between pairs of x-y data");
  return params;
}

PiecewiseLinear::PiecewiseLinear(const InputParameters & parameters)
  : PiecewiseLinearBase(parameters)
{
  buildInterpolation();
}
