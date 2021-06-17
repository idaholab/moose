//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallDistanceMixingLengthAux.h"

registerMooseObject("NavierStokesApp", WallDistanceMixingLengthAux);

InputParameters
WallDistanceMixingLengthAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the turbulent mixing length by assuming that it is "
                             "proportional to the distance from the nearest wall.");
  params.addParam<std::vector<BoundaryName>>("walls", "Boundaries that correspond to solid walls");
  params.addParam<Real>("von_karman_const", 0.4, "");
  return params;
}

WallDistanceMixingLengthAux::WallDistanceMixingLengthAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _von_karman_const(getParam<Real>("von_karman_const"))
{
  const MeshBase & mesh = _subproblem.mesh().getMesh();
  if (!mesh.is_replicated())
    mooseError("WallDistanceMixingLengthAux only supports replicated meshes");
}

Real
WallDistanceMixingLengthAux::computeValue()
{
  // Get references to the Moose and libMesh mesh objects
  const MooseMesh & m_mesh = _subproblem.mesh();
  const MeshBase & l_mesh = m_mesh.getMesh();

  // Get the ids of the wall boundaries
  std::vector<BoundaryID> vec_ids = m_mesh.getBoundaryIDs(_wall_boundary_names, true);

  // Loop over all boundaries
  Real min_sq_dist = 1e9;
  const auto & bnd_to_elem_map = m_mesh.getBoundariesToElems();
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
      Point bnd_pos = elem.centroid();
      Real sq_dist = (bnd_pos - _q_point[_qp]).norm_sq();
      min_sq_dist = std::min(min_sq_dist, sq_dist);
    }
  }

  // Return the mixing length
  return _von_karman_const * std::sqrt(min_sq_dist);
}
