//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionElementAverage.h"

registerMooseObject("MooseApp", FunctionElementAverage);

InputParameters
FunctionElementAverage::validParams()
{
  InputParameters params = FunctionElementIntegral::validParams();
  params.addClassDescription("Computes the average of a function over a volume.");
  return params;
}

FunctionElementAverage::FunctionElementAverage(const InputParameters & parameters)
  : FunctionElementIntegral(parameters)
{
}

void
FunctionElementAverage::initialize()
{
  FunctionElementIntegral::initialize();
  _volume = 0;
}

void
FunctionElementAverage::execute()
{
  FunctionElementIntegral::execute();
  _volume += _current_elem_volume;
}

Real
FunctionElementAverage::getValue()
{
  return _integral_value / _volume;
}

void
FunctionElementAverage::finalize()
{
  gatherSum(_volume);
  gatherSum(_integral_value);
}

void
FunctionElementAverage::threadJoin(const UserObject & y)
{
  FunctionElementIntegral::threadJoin(y);
  const FunctionElementAverage & pps = static_cast<const FunctionElementAverage &>(y);
  _volume += pps._volume;
}
