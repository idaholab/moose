//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredAverage.h"

registerMooseObject("MooseApp", LayeredAverage);

InputParameters
LayeredAverage::validParams()
{
  InputParameters params =
      LayeredVolumeAverageBase<ElementIntegralVariableUserObject>::validParams();
  params.addClassDescription("Computes averages of variables over layers");

  return params;
}

LayeredAverage::LayeredAverage(const InputParameters & parameters)
  : LayeredVolumeAverageBase<ElementIntegralVariableUserObject>(parameters)
{
}
