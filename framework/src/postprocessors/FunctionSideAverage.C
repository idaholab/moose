//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionSideAverage.h"

registerMooseObject("MooseApp", FunctionSideAverage);

InputParameters
FunctionSideAverage::validParams()
{
  InputParameters params = FunctionSideIntegral::validParams();
  params.addClassDescription("Computes the average of a function over a boundary.");
  return params;
}

FunctionSideAverage::FunctionSideAverage(const InputParameters & parameters)
  : FunctionSideIntegral(parameters)
{
}

void
FunctionSideAverage::initialize()
{
  FunctionSideIntegral::initialize();
  _volume = 0;
}

void
FunctionSideAverage::execute()
{
  FunctionSideIntegral::execute();
  _volume += _current_side_volume;
}

Real
FunctionSideAverage::getValue()
{
  return _integral_value / _volume;
}

void
FunctionSideAverage::threadJoin(const UserObject & y)
{
  FunctionSideIntegral::threadJoin(y);
  const FunctionSideAverage & pps = static_cast<const FunctionSideAverage &>(y);
  _volume += pps._volume;
}

void
FunctionSideAverage::finalize()
{
  gatherSum(_volume);
  gatherSum(_integral_value);
}
