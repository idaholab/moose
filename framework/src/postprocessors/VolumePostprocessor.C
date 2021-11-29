//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumePostprocessor.h"

registerMooseObject("MooseApp", VolumePostprocessor);

InputParameters
VolumePostprocessor::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the volume of a specified block");
  return params;
}

VolumePostprocessor::VolumePostprocessor(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters)
{
}

void
VolumePostprocessor::threadJoin(const UserObject & y)
{
  const VolumePostprocessor & pps = static_cast<const VolumePostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
VolumePostprocessor::computeQpIntegral()
{
  return 1.0;
}
