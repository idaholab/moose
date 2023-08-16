//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideAverage.h"

registerMooseObject("MooseApp", LayeredSideAverage);

InputParameters
LayeredSideAverage::validParams()
{
  InputParameters params = LayeredSideAverageBase<LayeredSideIntegral>::validParams();
  params.addClassDescription("Computes side averages of a variable storing partial sums for the "
                             "specified number of intervals in a direction (x,y,z).");
  return params;
}

LayeredSideAverage::LayeredSideAverage(const InputParameters & parameters)
  : LayeredSideAverageBase<LayeredSideIntegral>(parameters)
{
}
