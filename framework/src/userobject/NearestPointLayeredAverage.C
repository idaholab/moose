//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestPointLayeredAverage.h"
#include "LayeredAverage.h"

registerMooseObject("MooseApp", NearestPointLayeredAverage);

InputParameters
NearestPointLayeredAverage::validParams()
{
  InputParameters params =
      NearestPointBase<LayeredAverage, ElementIntegralVariableUserObject>::validParams();

  params.addClassDescription(
      "Computes averages of a variable storing partial sums for the specified number of intervals "
      "in a direction (x,y,z). Given a list of points this object computes the layered average "
      "closest to each one of those points.");

  return params;
}

NearestPointLayeredAverage::NearestPointLayeredAverage(const InputParameters & parameters)
  : NearestPointBase<LayeredAverage, ElementIntegralVariableUserObject>(parameters)
{
}
