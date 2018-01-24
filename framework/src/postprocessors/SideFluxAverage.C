//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFluxAverage.h"

template <>
InputParameters
validParams<SideFluxAverage>()
{
  InputParameters params = validParams<SideFluxIntegral>();
  return params;
}

SideFluxAverage::SideFluxAverage(const InputParameters & parameters)
  : SideFluxIntegral(parameters), _volume(0)
{
}

void
SideFluxAverage::initialize()
{
  SideIntegralVariablePostprocessor::initialize();
  _volume = 0;
}

void
SideFluxAverage::execute()
{
  SideIntegralVariablePostprocessor::execute();
  _volume += _current_side_volume;
}

Real
SideFluxAverage::getValue()
{
  Real integral = SideIntegralVariablePostprocessor::getValue();

  gatherSum(_volume);

  return integral / _volume;
}

void
SideFluxAverage::threadJoin(const UserObject & y)
{
  SideIntegralVariablePostprocessor::threadJoin(y);
  const SideFluxAverage & pps = static_cast<const SideFluxAverage &>(y);
  _volume += pps._volume;
}
