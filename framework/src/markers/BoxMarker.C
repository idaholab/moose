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

#include "BoxMarker.h"

template<>
InputParameters validParams<BoxMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<RealVectorValue>("bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>("top_right", "The bottom left point (in x,y,z with spaces in-between).");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>("inside", marker_states, "How to mark elements inside the box.");
  params.addRequiredParam<MooseEnum>("outside", marker_states, "How to mark elements outside the box.");

  return params;
}


BoxMarker::BoxMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _inside((MarkerValue)(int)parameters.get<MooseEnum>("inside")),
    _outside((MarkerValue)(int)parameters.get<MooseEnum>("outside")),
    _bounding_box(parameters.get<RealVectorValue>("bottom_left"), parameters.get<RealVectorValue>("top_right"))
{
}

Marker::MarkerValue
BoxMarker::computeElementMarker()
{
  RealVectorValue centroid = _current_elem->centroid();

  if (_bounding_box.contains_point(centroid))
    return _inside;

  return _outside;
}

