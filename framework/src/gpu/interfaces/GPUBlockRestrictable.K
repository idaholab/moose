//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockRestrictable.h"
#include "MooseMesh.h"

BlockRestrictable::BlockRestrictable(const BlockRestrictable & object)
  : BlockRestrictable(object._moose_object, false)
{
  _element_ids = object._element_ids;
  _element_side_ids = object._element_side_ids;
  _node_ids = object._node_ids;
}

void
BlockRestrictable::initializeGPUBlockRestrictable(const GPUMesh * mesh)
{
  auto & blocks = blockRestricted() ? blockIDs() : meshBlockIDs();

  std::set<dof_id_type> element_ids;
  std::set<dof_id_type> node_ids;

  for (auto block : blocks)
  {
    element_ids.insert(mesh->getGPUSubdomainElementIDs(block).begin(),
                       mesh->getGPUSubdomainElementIDs(block).end());
    node_ids.insert(mesh->getGPUSubdomainNodeIDs(block).begin(),
                    mesh->getGPUSubdomainNodeIDs(block).end());
  }

  _element_ids = element_ids;
  _node_ids = node_ids;

  if (_moose_object->isParamValid("_material_data_type") &&
      _moose_object->getParam<Moose::MaterialDataType>("_material_data_type") !=
          Moose::BLOCK_MATERIAL_DATA &&
      _moose_object->getParam<Moose::MaterialDataType>("_material_data_type") !=
          Moose::BOUNDARY_MATERIAL_DATA)
  {
    std::set<GPUPair<dof_id_type, unsigned int>> element_side_ids;

    for (auto block : blocks)
      for (const auto & [elem, id] : mesh->getGPUSubdomainElementMap(block))
        for (unsigned int side = 0; side < elem->n_sides(); ++side)
          element_side_ids.insert({id, side});

    _element_side_ids = element_side_ids;
  }
}
