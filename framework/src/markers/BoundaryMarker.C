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
  params.addParam<Real>("distance", 0.0, "Distance from the boundary to refine within");
  return params;
}

BoundaryMarker::BoundaryMarker(const InputParameters & parameters)
  : Marker(parameters),
    _distance(getParam<Real>("distance")),
    _bnd_elem_ids(_mesh.getBoundariesToActiveSemiLocalElemIds()),
    _mark(parameters.get<MooseEnum>("mark").getEnum<MarkerValue>()),
    _boundary(_mesh.getBoundaryID(getParam<BoundaryName>("next_to")))
{
  if (_mesh.isDistributedMesh() && _distance > 0)
    mooseWarning("Elements with in `distance ` of a boundary segment on a different processor "
                 "might not get marked when running with a distributed mesh.");
}

Marker::MarkerValue
BoundaryMarker::computeElementMarker()
{
  if (_distance == 0.0)
  {
    if (_mesh.isBoundaryElem(_current_elem->id(), _boundary))
      return _mark;

    return DONT_MARK;
  }
  else
  {
    const auto it = _bnd_elem_ids.find(_boundary);
    if (it != _bnd_elem_ids.end())
      for (const auto id : it->second)
      {
        const auto elem = _mesh.elemPtr(id);
        const auto r = _current_elem->vertex_average() - elem->vertex_average();
        if (r.norm() < _distance)
          return _mark;
      }

    return DONT_MARK;
  }
}
