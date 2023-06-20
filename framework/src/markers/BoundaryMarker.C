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
  params.addRequiredParam<std::vector<BoundaryName>>("next_to",
                                                     "Boundaries to refine elements along");
  params.addParam<Real>("distance", 0.0, "Distance from the boundary to refine within");
  return params;
}

BoundaryMarker::BoundaryMarker(const InputParameters & parameters)
  : Marker(parameters),
    _distance(getParam<Real>("distance")),
    _bnd_elem_ids(_mesh.getBoundariesToActiveSemiLocalElemIds()),
    _mark(parameters.get<MooseEnum>("mark").getEnum<MarkerValue>()),
    _boundary_ids(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("next_to")))
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
    // is the current element member of any selected boundary element set?
    for (const auto boundary : _boundary_ids)
      if (_mesh.isBoundaryElem(_current_elem->id(), boundary))
        return _mark;

    return DONT_MARK;
  }
  else
  {
    for (const auto boundary : _boundary_ids)
    {
      const auto it = _bnd_elem_ids.find(boundary);
      if (it != _bnd_elem_ids.end())
        for (const auto id : it->second)
        {
          // shortcut if we are checing the current element itself
          if (id == _current_elem->id())
            return _mark;

          // otherwise compute distance to the boundary elements
          const auto elem = _mesh.elemPtr(id);
          const auto r = _current_elem->vertex_average() - elem->vertex_average();
          if (r.norm() < _distance)
            return _mark;
        }
    }
    return DONT_MARK;
  }
}
