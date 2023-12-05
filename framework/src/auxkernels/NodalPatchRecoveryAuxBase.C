//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecoveryAuxBase.h"

InputParameters
NodalPatchRecoveryAuxBase::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("This Auxkernel solves a least squares problem at each node to fit a "
                             "value from quantities defined on quadrature points.");
  params.addRequiredParam<UserObjectName>(
      "nodal_patch_recovery_uo",
      "The name of the userobject that sets up the least squares problem of the nodal patch.");
  return params;
}

NodalPatchRecoveryAuxBase::NodalPatchRecoveryAuxBase(const InputParameters & parameters)
  : AuxKernel(parameters)
{
  if (!isNodal())
    mooseError(name(), " only runs on nodal variables.");
}

Real
NodalPatchRecoveryAuxBase::computeValue()
{
  // get node-to-conneted-elem map
  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Missing entry in node to elem map");

  _elem_ids.clear();
  blockRestrictElements(_elem_ids, node_to_elem_pair->second);

  // consider the case for corner node
  if (_elem_ids.size() == 1)
  {
    const dof_id_type elem_id = _elem_ids[0];
    for (auto & n : _mesh.elemPtr(elem_id)->node_ref_range())
    {
      node_to_elem_pair = node_to_elem_map.find(n.id());
      std::vector<dof_id_type> elem_ids_candidate = node_to_elem_pair->second;
      if (elem_ids_candidate.size() > _elem_ids.size())
      {
        std::vector<dof_id_type> elem_ids_candidate_restricted;
        blockRestrictElements(elem_ids_candidate_restricted, elem_ids_candidate);

        if (elem_ids_candidate_restricted.size() > _elem_ids.size())
          _elem_ids = elem_ids_candidate_restricted;
      }
    }
  }

  // get the value from a userobject (overridden in derived class)
  return nodalPatchRecovery();
}

void
NodalPatchRecoveryAuxBase::blockRestrictElements(
    std::vector<dof_id_type> & elem_ids,
    const std::vector<dof_id_type> & node_to_elem_pair_elems) const
{
  if (blockRestricted())
    for (auto elem_id : node_to_elem_pair_elems)
    {
      for (const auto block_id : blockIDs())
        if (block_id == _mesh.elemPtr(elem_id)->subdomain_id())
          elem_ids.push_back(elem_id);
    }
  else
    elem_ids = node_to_elem_pair_elems;
}
