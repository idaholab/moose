//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallDistanceAux.h"

registerMooseObject("NavierStokesApp", WallDistanceAux);

InputParameters
WallDistanceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the distance from the nearest wall.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  return params;
}

WallDistanceAux::WallDistanceAux(const InputParameters & parameters)
  : AuxKernel(parameters), _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  const MeshBase & mesh = _subproblem.mesh().getMesh();
  if (!mesh.is_replicated())
    mooseError("WallDistanceAux only supports replicated meshes");
  if (!dynamic_cast<MooseVariableFV<Real> *>(&_var))
    paramError("variable",
               "'",
               name(),
               "' is currently programmed to use finite volume machinery, so make sure that '",
               _var.name(),
               "' is a finite volume variable.");
}

Real
WallDistanceAux::computeValue()
{
  // Get reference to the libMesh mesh object
  const MeshBase & l_mesh = _mesh.getMesh();

  // Get the ids of the wall boundaries
  std::vector<BoundaryID> vec_ids = _mesh.getBoundaryIDs(_wall_boundary_names, true);

  // Loop over all boundaries
  Real min_dist2 = 1e9;
  const auto & bnd_to_elem_map = _mesh.getBoundariesToActiveSemiLocalElemIds();
  for (BoundaryID bid : vec_ids)
  {
    // Get the set of elements on this boundary
    auto search = bnd_to_elem_map.find(bid);
    if (search == bnd_to_elem_map.end())
      mooseError("Error computing wall distance; the boundary id ", bid, " is invalid");
    const auto & bnd_elems = search->second;

    // Loop over all boundary elements and find the distance to the closest one
    for (dof_id_type elem_id : bnd_elems)
    {
      const Elem & elem{l_mesh.elem_ref(elem_id)};
      const auto side = _mesh.sideWithBoundaryID(&elem, bid);
      const FaceInfo * fi = _mesh.faceInfo(&elem, side);
      // It's possible that we are on an internal boundary
      if (!fi)
      {
        const Elem * const neigh = elem.neighbor_ptr(side);
        mooseAssert(neigh,
                    "In WallDistanceAux, we could not find a face information object with elem "
                    "and side, and we are on an external boundary. This shouldn't happen.");
        const auto neigh_side = neigh->which_neighbor_am_i(&elem);
        fi = _mesh.faceInfo(neigh, neigh_side);
        mooseAssert(fi, "We should have a face info for either the elem or neigh side");
      }
      Point bnd_pos = fi->faceCentroid();
      const auto distance = bnd_pos - _q_point[_qp];
      const auto dist2 = distance * distance;
      mooseAssert(dist2 != 0, "This distance should never be 0");
      min_dist2 = std::min(min_dist2, dist2);
    }
  }

  return std::sqrt(min_dist2);
}
