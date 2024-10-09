//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallNormalUnitVectorAux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("NavierStokesApp", WallNormalUnitVectorAux);

InputParameters
WallNormalUnitVectorAux::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription("Computes unit normal vector to the nearest wall for every cell.");
  params.addRequiredParam<std::vector<BoundaryName>>("walls",
                                                     "Boundaries that correspond to solid walls.");
  return params;
}

WallNormalUnitVectorAux::WallNormalUnitVectorAux(const InputParameters & parameters)
  : VectorAuxKernel(parameters), _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  const MeshBase & mesh = _subproblem.mesh().getMesh();
  if (!mesh.is_replicated())
    mooseError("WallNormalUnitVectorAux only supports replicated meshes");

  if (_wall_boundary_names.empty())
    paramError("walls",
               "At least one wall boundary needs to be specifid to which the wall normals will be "
               "computed for each cell.");
}

RealVectorValue
WallNormalUnitVectorAux::computeValue()
{
  // Get reference to the libMesh mesh object
  const MeshBase & l_mesh = _mesh.getMesh();

  // Get the ids of the wall boundaries
  std::vector<BoundaryID> vec_ids = _mesh.getBoundaryIDs(_wall_boundary_names, true);

  // Unit vector to the nearest wall
  Point direction_vector(0.0);

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
      if (dist2 < min_dist2)
      {
        mooseAssert(dist2 != 0, "This distance should never be 0");
        min_dist2 = std::min(min_dist2, dist2);
        direction_vector = distance;
      }
    }
  }

  return direction_vector / direction_vector.norm();
}
