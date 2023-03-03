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
registerMooseObjectRenamed("MooseApp", ADPiecewiseLinear, "02/03/2024 00:00", PiecewiseLinear);

InputParameters
PiecewiseLinear::validParams()
{
  InputParameters params = PiecewiseLinearBase::validParams();
  params.addParam<bool>(
      "extrap", false, "If true, extrapolates when sample point is outside of abscissa range");
  params.addClassDescription("Linearly interpolates between pairs of x-y data");
  return params;
}

PiecewiseLinear::PiecewiseLinear(const InputParameters & parameters)
  : PiecewiseLinearBase(parameters)
{
  this->buildInterpolation(this->template getParam<bool>("extrap"));
}
