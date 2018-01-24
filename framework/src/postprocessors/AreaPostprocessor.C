//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AreaPostprocessor.h"

template <>
InputParameters
validParams<AreaPostprocessor>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  return params;
}

AreaPostprocessor::AreaPostprocessor(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters)
{
}

void
AreaPostprocessor::threadJoin(const UserObject & y)
{
  const AreaPostprocessor & pps = static_cast<const AreaPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
AreaPostprocessor::computeQpIntegral()
{
  return 1.0;
}
