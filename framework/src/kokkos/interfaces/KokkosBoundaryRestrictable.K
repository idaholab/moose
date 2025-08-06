//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMaterialBase.h"

#include "BoundaryRestrictable.h"

#include "MooseMesh.h"

BoundaryRestrictable::BoundaryRestrictable(const BoundaryRestrictable & object)
  : BoundaryRestrictable(&object._moose_object, object._bnd_nodal)
{
  _kokkos_node_ids = object._kokkos_node_ids;
  _kokkos_element_side_ids = object._kokkos_element_side_ids;
}

void
BoundaryRestrictable::initializeKokkosBoundaryRestrictable(MooseMesh * mesh)
{
  if (_moose_object.isParamValid("_kokkos_material"))
    return;

  auto & boundaries = boundaryRestricted() ? boundaryIDs() : meshBoundaryIDs();

  auto kokkos_mesh = mesh->getKokkosMesh();

  if (_bnd_nodal)
  {
    std::set<dof_id_type> node_ids;

    for (auto boundary : boundaries)
      for (const auto node : as_range(mesh->getMesh().bid_nodes_begin(boundary),
                                      mesh->getMesh().bid_nodes_end(boundary)))
        if (node->processor_id() == _moose_object.processor_id())
          node_ids.insert(kokkos_mesh->getNodeID(node));

    _kokkos_node_ids = node_ids;
  }
  else
  {
    std::set<Moose::Kokkos::Pair<dof_id_type, unsigned int>> element_side_ids;

    for (auto boundary : boundaries)
      for (auto elem_id : mesh->getBoundaryActiveSemiLocalElemIds(boundary))
      {
        const auto elem = mesh->elemPtr(elem_id);

        if (elem->processor_id() == _moose_object.processor_id())
          element_side_ids.insert(
              {kokkos_mesh->getElementID(elem), mesh->sideWithBoundaryID(elem, boundary)});
      }

    _kokkos_element_side_ids = element_side_ids;
  }
}
