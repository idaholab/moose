//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * Creates a box of specified width, length and height,
 * with its center at specified position,
 * and with the direction along the width direction specified,
 * and with the direction along the length direction specified.
 * Then elements are marked as inside or outside this box
 */

#include "OrientedBoxMarker.h"
#include "OrientedBoxInterface.h"

registerMooseObject("MooseApp", OrientedBoxMarker);

InputParameters
OrientedBoxMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params += OrientedBoxInterface::validParams();

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements inside the box.");
  params.addRequiredParam<MooseEnum>(
      "outside", marker_states, "How to mark elements outside the box.");

  params.addClassDescription(
      "Marks inside and outside a box that can have arbitrary orientation and center point.");
  return params;
}

OrientedBoxMarker::OrientedBoxMarker(const InputParameters & parameters)
  : Marker(parameters),
    OrientedBoxInterface(parameters),
    _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
    _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside"))
{
}

/**
 * Marks elements inside and outside the box
 */
Marker::MarkerValue
OrientedBoxMarker::computeElementMarker()
{
  Point centroid = _current_elem->vertex_average();

  if (containsPoint(centroid))
    return _inside;

  return _outside;
}
