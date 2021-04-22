//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryMarker.h"

registerMooseObject("MooseApp", BoundaryMarker);

InputParameters
BoundaryMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addClassDescription(
      "Marks all elements with sides on a given boundary for refinement/coarsening");
  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "mark", marker_states, "How to mark elements adjacent to the boundary.");
  params.addRequiredParam<BoundaryName>("next_to", "Boundary to refine elements along");
  return params;
}

BoundaryMarker::BoundaryMarker(const InputParameters & parameters)
  : Marker(parameters),
    _mark(parameters.get<MooseEnum>("mark").getEnum<MarkerValue>()),
    _boundary(_mesh.getBoundaryID(getParam<BoundaryName>("next_to")))
{
}

Marker::MarkerValue
BoundaryMarker::computeElementMarker()
{
  const auto nsides = _current_elem->n_sides();

  for (unsigned int side = 0; side < nsides; ++side)
  {
    auto ids = _mesh.getBoundaryIDs(_current_elem, side);
    for (auto id : ids)
      if (id == _boundary)
        return _mark;
  }

  return DONT_MARK;
}
