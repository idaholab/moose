//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AreaPostprocessor.h"

registerMooseObject("MooseApp", AreaPostprocessor);

InputParameters
AreaPostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addClassDescription("Computes the \"area\" or dimension - 1 \"volume\" of a given "
                             "boundary or boundaries in your mesh.");
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
