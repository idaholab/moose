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
#include "MooseMesh.h"

template <>
InputParameters
validParams<CircleMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<Point>("point", "The center of the circle.");
  params.addRequiredParam<Real>("radius",
                                "Distance from the center of the circle to mark elements");
  params.addCoupledVar(
      "periodic_variable",
      "Use periodicity settings of this variable to calculate distance from the element.");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements inside the circle.");
  params.addRequiredParam<MooseEnum>(
      "outside", marker_states, "How to mark elements outside the circle.");

  return params;
}

CircleMarker::CircleMarker(const InputParameters & parameters)
  : Marker(parameters),
    Coupleable(this, false),
    _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
    _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside")),
    _p(getParam<Point>("point")),
    _r(getParam<Real>("radius")),
    _periodic_variable(isCoupled("periodic_variable") ? coupled("periodic_variable") : -1)

{
}

Marker::MarkerValue
CircleMarker::computeElementMarker()
{
  Point centroid = _current_elem->centroid();

  Real distance = _periodic_variable < 0
                      ? (centroid - _p).norm()
                      : _mesh.minPeriodicDistance(_periodic_variable, centroid, _p);

  if (distance < _r)
    return _inside;

  return _outside;
}
