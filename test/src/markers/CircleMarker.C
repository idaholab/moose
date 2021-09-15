//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CircleMarker.h"

registerMooseObject("MooseTestApp", CircleMarker);

InputParameters
CircleMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<Point>("point", "The center of the circle.");
  params.addRequiredParam<Real>("radius",
                                "Distance from the center of the circle to mark elements");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements inside the circle.");
  params.addRequiredParam<MooseEnum>(
      "outside", marker_states, "How to mark elements outside the circle.");

  return params;
}

CircleMarker::CircleMarker(const InputParameters & parameters)
  : Marker(parameters),
    _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
    _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside")),
    _p(getParam<Point>("point")),
    _r(getParam<Real>("radius"))
{
}

Marker::MarkerValue
CircleMarker::computeElementMarker()
{
  Point centroid = _current_elem->vertex_average();

  if ((centroid - _p).norm() < _r)
    return _inside;

  return _outside;
}
