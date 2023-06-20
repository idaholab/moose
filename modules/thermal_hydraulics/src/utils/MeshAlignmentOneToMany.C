//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignmentOneToMany.h"

MeshAlignmentOneToMany::MeshAlignmentOneToMany(const MooseMesh & mesh)
  : MeshAlignmentBase(mesh), _max_coupling_size(0)
{
}

bool
MeshAlignmentOneToMany::hasCoupledPrimaryElemID(const dof_id_type & secondary_elem_id) const
{
  return _secondary_elem_id_to_primary_elem_id.find(secondary_elem_id) !=
         _secondary_elem_id_to_primary_elem_id.end();
}

dof_id_type
MeshAlignmentOneToMany::getCoupledPrimaryElemID(const dof_id_type & secondary_elem_id) const
{
  mooseAssert(hasCoupledPrimaryElemID(secondary_elem_id),
              "The element ID has no coupled elements.");
  return _secondary_elem_id_to_primary_elem_id.find(secondary_elem_id)->second;
}

bool
MeshAlignmentOneToMany::hasCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const
{
  return _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id) !=
         _primary_elem_id_to_secondary_elem_ids.end();
}

const std::vector<dof_id_type> &
MeshAlignmentOneToMany::getCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const
{
  mooseAssert(hasCoupledSecondaryElemIDs(primary_elem_id),
              "The element ID has no coupled elements.");
  return _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id)->second;
}

unsigned int
MeshAlignmentOneToMany::getCoupledPrimaryElemQpIndex(const dof_id_type & secondary_elem_id,
                                                     const unsigned int & secondary_qp) const
{
  auto it = _secondary_elem_id_to_qp_indices.find(secondary_elem_id);
  mooseAssert(it != _secondary_elem_id_to_qp_indices.end(),
              "The element ID has no coupled quadrature point indices.");
  mooseAssert(secondary_qp < it->second.size(), "The quadrature index does not exist in the map.");
  return it->second[secondary_qp];
}
