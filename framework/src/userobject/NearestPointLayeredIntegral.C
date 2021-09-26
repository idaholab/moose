//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestPointLayeredIntegral.h"
#include "LayeredIntegral.h"

registerMooseObject("MooseApp", NearestPointLayeredIntegral);

defineLegacyParams(NearestPointLayeredIntegral);

InputParameters
NearestPointLayeredIntegral::validParams()
{
  InputParameters params =
      nearestPointBaseValidParams<LayeredIntegral, ElementIntegralVariableUserObject>();
  params.addClassDescription(
      "Computes integrals of a variable storing partial sums for the specified number of intervals "
      "in a direction (x,y,z). Given a list of points this object computes the layered integral "
      "closest to each one of those points.");
  return params;
}

NearestPointLayeredIntegral::NearestPointLayeredIntegral(const InputParameters & parameters)
  : NearestPointBase<LayeredIntegral, ElementIntegralVariableUserObject>(parameters)
{
}

const std::vector<Point>
NearestPointLayeredIntegral::spatialPoints() const
{
  std::vector<Point> points;

  for (MooseIndex(_points) i = 0; i < _points.size(); ++i)
  {
    const auto & layers = _user_objects[i]->getLayerCenters();
    auto direction = _user_objects[i]->direction();

    for (const auto & l : layers)
    {
      Point pt = _points[i];
      pt(direction) = l;
      points.push_back(pt);
    }
  }

  return points;
}
