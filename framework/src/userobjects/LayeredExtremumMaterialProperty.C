//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredExtremumMaterialProperty.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", LayeredExtremumMaterialProperty);

InputParameters
LayeredExtremumMaterialProperty::validParams()
{
  InputParameters params = ElementExtremeMaterialProperty::validParams();
  params += LayeredBase::validParams();
  params.addClassDescription("Compute material property extrema over layers.");
  params.registerBase("UserObject");

  return params;
}

LayeredExtremumMaterialProperty::LayeredExtremumMaterialProperty(const InputParameters & parameters)
  : ElementExtremeMaterialProperty(parameters), LayeredBase(parameters)
{
}

void
LayeredExtremumMaterialProperty::initialize()
{
  ElementExtremeMaterialProperty::initialize();
  LayeredBase::initialize();

  // Initialize layer values
  for (auto layer : make_range(_num_layers))
    setLayerValue(
        layer, _type == MIN ? std::numeric_limits<Real>::max() : -std::numeric_limits<Real>::max());
}

void
LayeredExtremumMaterialProperty::execute()
{
  // Do not keep track of the layer extremum with _value
  ElementExtremeMaterialProperty::initialize();
  ElementExtremeMaterialProperty::execute();
  unsigned int layer = getLayer(_current_elem->vertex_average());

  setLayerValue(layer, extreme_value(getLayerValue(layer), _value));
}

Real
LayeredExtremumMaterialProperty::extreme_value(Real a, Real b) const
{
  if (_type == MIN)
    return std::min(a, b);
  else
    return std::max(a, b);
}

void
LayeredExtremumMaterialProperty::finalize()
{
  LayeredBase::finalize();
}

void
LayeredExtremumMaterialProperty::threadJoin(const UserObject & y)
{
  ElementExtremeMaterialProperty::threadJoin(y);
  LayeredBase::threadJoin(y);
}

const std::vector<Point>
LayeredExtremumMaterialProperty::spatialPoints() const
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
