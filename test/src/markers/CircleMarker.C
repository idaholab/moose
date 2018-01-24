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

#include "CircleMarker.h"

template <>
InputParameters
validParams<CircleMarker>()
{
  InputParameters params = validParams<Marker>();
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
  Point centroid = _current_elem->centroid();

  if ((centroid - _p).size() < _r)
    return _inside;

  return _outside;
}
