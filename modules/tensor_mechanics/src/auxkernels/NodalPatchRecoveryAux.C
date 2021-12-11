//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecoveryAux.h"
#include "NodalPatchRecoveryBase.h"

registerMooseObject("TensorMechanicsApp", NodalPatchRecoveryAux);

InputParameters
NodalPatchRecoveryAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("This Auxkernel solves a least squares problem at each node to fit a "
                             "value from quantities defined on quadrature points.");
  params.addRequiredParam<UserObjectName>(
      "nodal_patch_recovery_uo",
      "The name of the userobject that sets up the least squares problem of the nodal patch.");
  return params;
}

NodalPatchRecoveryAux::NodalPatchRecoveryAux(const InputParameters & parameters)
  : AuxKernel(parameters), _npr(getUserObject<NodalPatchRecoveryBase>("nodal_patch_recovery_uo"))
{
}

Real
NodalPatchRecoveryAux::computeValue()
{
  if (!isNodal())
    mooseError(name(), " only runs on nodal variables.");

  // get node-to-conneted-elem map
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Missing entry in node to elem map");
  std::vector<dof_id_type> elem_ids = node_to_elem_pair->second;

  // consider the case for corner node
  if (elem_ids.size() == 1)
  {
    dof_id_type elem_id = elem_ids[0];
    for (auto & n : _mesh.elemPtr(elem_id)->node_ref_range())
    {
      node_to_elem_pair = node_to_elem_map.find(n.id());
      std::vector<dof_id_type> elem_ids_candidate = node_to_elem_pair->second;
      if (elem_ids_candidate.size() > elem_ids.size())
        elem_ids = elem_ids_candidate;
    }
  }

  return _npr.nodalPatchRecovery(*_current_node, elem_ids);
}
