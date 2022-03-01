//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoxMarker.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", BoxMarker);

InputParameters
BoxMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<RealVectorValue>(
      "bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>(
      "top_right", "The bottom left point (in x,y,z with spaces in-between).");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements inside the box.");
  params.addRequiredParam<MooseEnum>(
      "outside", marker_states, "How to mark elements outside the box.");

  params.addClassDescription(
      "Marks the region inside and outside of a 'box' domain for refinement or coarsening.");
  return params;
}

BoxMarker::BoxMarker(const InputParameters & parameters)
  : Marker(parameters),
    _inside(parameters.get<MooseEnum>("inside").getEnum<MarkerValue>()),
    _outside(parameters.get<MooseEnum>("outside").getEnum<MarkerValue>()),
    _bounding_box(MooseUtils::buildBoundingBox(parameters.get<RealVectorValue>("bottom_left"),
                                               parameters.get<RealVectorValue>("top_right")))
{
}

Marker::MarkerValue
BoxMarker::computeElementMarker()
{
  RealVectorValue centroid = _current_elem->vertex_average();

  if (_bounding_box.contains_point(centroid))
    return _inside;

  return _outside;
}
