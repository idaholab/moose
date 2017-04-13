/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

/**
 * Creates a box of specified width, length and height,
 * with its center at specified position,
 * and with the direction along the width direction specified,
 * and with the direction along the length direction specified.
 * Then elements are marked as inside or outside this box
 */

#include "OrientedBoxMarker.h"
#include "OrientedBoxInterface.h"

template <>
InputParameters
validParams<OrientedBoxMarker>()
{
  InputParameters params = validParams<Marker>();
  params += validParams<OrientedBoxInterface>();

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
  Point centroid = _current_elem->centroid();

  if (containsPoint(centroid))
    return _inside;

  return _outside;
}
