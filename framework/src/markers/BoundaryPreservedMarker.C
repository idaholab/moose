//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryPreservedMarker.h"
#include "MooseMeshUtils.h"

#include "libmesh/error_vector.h"

registerMooseObject("MooseApp", BoundaryPreservedMarker);

InputParameters
BoundaryPreservedMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<BoundaryName>(
      "preserved_boundary",
      "The name of the boundary to be preserved. Will try to preserve the boundary during AMR");
  params.addRequiredParam<MarkerName>(
      "marker", "The marker name to decide whether to carsen or refine elements.");
  params.addClassDescription("Marks elements for refinement or coarsening based on the provided "
                             "marker value, while preserving the given boundary.");
  return params;
}

BoundaryPreservedMarker::BoundaryPreservedMarker(const InputParameters & parameters)
  : Marker(parameters),
    _marker_name(parameters.get<MarkerName>("marker")),
    _marker(&getMarkerValue(_marker_name))
{
  _mesh.errorIfDistributedMesh(type());
  BoundaryName boundary_name = getParam<BoundaryName>("preserved_boundary");
  auto boundary_ids = MooseMeshUtils::getBoundaryIDs(_mesh, {boundary_name}, true);
  mooseAssert(boundary_ids.size() == 1, "Boundary does not exist");
  _preserved_boundary = boundary_ids[0];
}

bool
BoundaryPreservedMarker::preserveBoundary(const Elem * const & current_elem)
{
  auto & elem_side_bnd_ids = _mesh.getMesh().get_boundary_info().get_sideset_map();

  // Do not coarsen the elements when they are connected to the preserved boundary
  for (const auto & pr : as_range(elem_side_bnd_ids.equal_range(current_elem)))
    if (pr.second.second == _preserved_boundary)
      return true;

  return false;
}

Marker::MarkerValue
BoundaryPreservedMarker::computeElementMarker()
{
  MarkerValue marker_value = static_cast<MarkerValue>((*_marker)[0]);
  if (marker_value == COARSEN && preserveBoundary(_current_elem))
    return DO_NOTHING;
  else
    return marker_value;
}
