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

#include "TwoCircleMarker.h"

template <>
InputParameters
validParams<TwoCircleMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<Point>("point1", "The center of the first circle.");
  params.addRequiredParam<Real>("radius1",
                                "Distance from the center of the first circle to mark elements.");
  params.addRequiredParam<Point>("point2", "The center of the second circle.");
  params.addRequiredParam<Real>("radius2",
                                "Distance from the center of the second circle to mark elements.");
  params.addParam<Real>("shut_off_time",
                        std::numeric_limits<Real>::max(),
                        "Time at which the second circle becomes inactive.");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements inside the circles.");
  params.addRequiredParam<MooseEnum>(
      "outside", marker_states, "How to mark elements outside the circles.");

  return params;
}

TwoCircleMarker::TwoCircleMarker(const InputParameters & parameters)
  : Marker(parameters),
    _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
    _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside")),
    _p1(getParam<Point>("point1")),
    _r1(getParam<Real>("radius1")),
    _p2(getParam<Point>("point2")),
    _r2(getParam<Real>("radius2")),
    _shut_off_time(getParam<Real>("shut_off_time"))
{
}

Marker::MarkerValue
TwoCircleMarker::computeElementMarker()
{
  Point centroid = _current_elem->centroid();

  if (((centroid - _p1).size() < _r1) ||
      (((centroid - _p2).size() < _r2) && (_fe_problem.time() < _shut_off_time)))
    return _inside;

  return _outside;
}
