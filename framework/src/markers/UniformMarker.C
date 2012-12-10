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

#include "UniformMarker.h"

template<>
InputParameters validParams<UniformMarker>()
{
  InputParameters params = validParams<Marker>();
  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>("mark", marker_states, "How to mark elements.");

  return params;
}


UniformMarker::UniformMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _mark((MarkerValue)(int)parameters.get<MooseEnum>("mark"))
{
}

Marker::MarkerValue
UniformMarker::computeElementMarker()
{
  return _mark;
}

