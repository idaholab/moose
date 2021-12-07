//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionLayeredIntegral.h"

registerMooseObject("MooseApp", FunctionLayeredIntegral);

InputParameters
FunctionLayeredIntegral::validParams()
{
  InputParameters params = FunctionElementIntegralUserObject::validParams();
  params += LayeredBase::validParams();
  params.addClassDescription("Integrates a function in layers");
  return params;
}

FunctionLayeredIntegral::FunctionLayeredIntegral(const InputParameters & parameters)
  : FunctionElementIntegralUserObject(parameters), LayeredBase(parameters)
{
}

void
FunctionLayeredIntegral::initialize()
{
  FunctionElementIntegralUserObject::initialize();
  LayeredBase::initialize();
}

void
FunctionLayeredIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->vertex_average());

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

void
FunctionLayeredIntegral::finalize()
{
  LayeredBase::finalize();
}

void
FunctionLayeredIntegral::threadJoin(const UserObject & y)
{
  FunctionElementIntegralUserObject::threadJoin(y);
  LayeredBase::threadJoin(y);
}

const std::vector<Point>
FunctionLayeredIntegral::spatialPoints() const
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
