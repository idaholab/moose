//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideIntegral.h"

registerMooseObject("MooseApp", LayeredSideIntegral);

InputParameters
LayeredSideIntegral::validParams()
{
  InputParameters params = SideIntegralVariableUserObject::validParams();
  params += LayeredBase::validParams();
  params.addClassDescription("Computes surface integral of a variable storing partial sums for the "
                             "specified number of intervals in a direction (x,y,z).");
  return params;
}

LayeredSideIntegral::LayeredSideIntegral(const InputParameters & parameters)
  : SideIntegralVariableUserObject(parameters), LayeredBase(parameters)
{
  if (parameters.isParamValid("block") && parameters.isParamValid("boundary"))
    mooseError("Both block and boundary cannot be specified in LayeredSideIntegral. If you want to "
               "define the geometric bounds of the layers from a specified block set "
               "layer_bounding_block instead.");
}

void
LayeredSideIntegral::initialize()
{
  SideIntegralVariableUserObject::initialize();
  LayeredBase::initialize();
}

void
LayeredSideIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->vertex_average());

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

void
LayeredSideIntegral::finalize()
{
  LayeredBase::finalize();
}

void
LayeredSideIntegral::threadJoin(const UserObject & y)
{
  SideIntegralVariableUserObject::threadJoin(y);
  LayeredBase::threadJoin(y);
}

const std::vector<Point>
LayeredSideIntegral::spatialPoints() const
{
  std::vector<Point> points;

  for (const auto & l : _layer_centers)
  {
    Point pt(0.0, 0.0, 0.0);
    pt(_direction) = l;
    points.push_back(pt);
  }

  return points;
}
