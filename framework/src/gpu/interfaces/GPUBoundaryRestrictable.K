//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMaterialBase.h"

#include "BoundaryRestrictable.h"

#include "MooseMesh.h"

BoundaryRestrictable::BoundaryRestrictable(const BoundaryRestrictable & object)
  : BoundaryRestrictable(&object._moose_object, object._bnd_nodal, false)
{
  _node_ids = object._node_ids;
  _element_side_ids = object._element_side_ids;
}

void
BoundaryRestrictable::initializeGPUBoundaryRestrictable(MooseMesh * mesh)
{
  if (_moose_object.isParamValid("_gpu_material"))
    return;

  auto & boundaries = boundaryRestricted() ? boundaryIDs() : meshBoundaryIDs();

  auto gpu_mesh = mesh->getGPUMesh();

  if (_bnd_nodal)
  {
    std::set<dof_id_type> node_ids;

    for (auto boundary : boundaries)
      for (const auto node : as_range(mesh->getMesh().bid_nodes_begin(boundary),
                                      mesh->getMesh().bid_nodes_end(boundary)))
        if (node->processor_id() == _moose_object.processor_id())
          node_ids.insert(gpu_mesh->getGPUNodeID(node));

    _node_ids = node_ids;
  }
  else
  {
    std::set<GPUPair<dof_id_type, unsigned int>> element_side_ids;

    for (auto boundary : boundaries)
      for (auto elem_id : mesh->getBoundaryActiveSemiLocalElemIds(boundary))
      {
        const auto elem = mesh->elemPtr(elem_id);

        if (elem->processor_id() == _moose_object.processor_id())
          element_side_ids.insert(
              {gpu_mesh->getGPUElementID(elem), mesh->sideWithBoundaryID(elem, boundary)});
      }

    _element_side_ids = element_side_ids;
  }
}
