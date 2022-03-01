//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UniformMarker.h"

registerMooseObject("MooseApp", UniformMarker);

InputParameters
UniformMarker::validParams()
{
  InputParameters params = Marker::validParams();
  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>("mark", marker_states, "How to mark elements.");
  params.addClassDescription("Uniformly mark all elements for refinement or coarsening.");
  return params;
}

UniformMarker::UniformMarker(const InputParameters & parameters)
  : Marker(parameters), _mark((MarkerValue)(int)parameters.get<MooseEnum>("mark"))
{
}

Marker::MarkerValue
UniformMarker::computeElementMarker()
{
  return _mark;
}
