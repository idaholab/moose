//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestRadiusLayeredAverage.h"
#include "LayeredAverage.h"

registerMooseObject("MooseApp", NearestRadiusLayeredAverage);

InputParameters
NearestRadiusLayeredAverage::validParams()
{
  InputParameters params =
      NearestPointBase<LayeredAverage, ElementIntegralVariableUserObject>::validParams();

  // this object sets 'dist_norm' so the user shouldn't input it
  params.set<MooseEnum>("dist_norm") = "radius";
  params.suppressParameter<MooseEnum>("dist_norm");

  // 'direction' in this case corresponds to 'axis' so the user shouldn't input it
  params.set<MooseEnum>("axis") = params.get<MooseEnum>("direction");
  params.suppressParameter<MooseEnum>("axis");

  params.addClassDescription(
      "Computes averages of a variable storing partial sums for the specified number of intervals "
      "in a direction (x,y,z). Given a list of points this object computes the layered average "
      "closest to each one of those points, where the distance is computed in terms of radius (or "
      "distance to the origin in the plane perpendicular to 'direction').");

  return params;
}

NearestRadiusLayeredAverage::NearestRadiusLayeredAverage(const InputParameters & parameters)
  : NearestPointBase<LayeredAverage, ElementIntegralVariableUserObject>(parameters)
{
}
