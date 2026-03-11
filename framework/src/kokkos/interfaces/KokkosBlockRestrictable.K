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

BlockRestrictable::BlockRestrictable(const BlockRestrictable & object,
                                     const Moose::Kokkos::FunctorCopy &)
  : _blk_dual_restrictable(object._blk_dual_restrictable),
    _blk_feproblem(object._blk_feproblem),
    _blk_mesh(object._blk_mesh),
    _boundary_ids(object._boundary_ids),
    _blk_tid(object._blk_tid),
    _blk_name(object._blk_name),
    _blk_dim(object._blk_dim),
    _moose_object(object._moose_object)
{
  _kokkos_element_ids = object._kokkos_element_ids;
  _kokkos_node_ids = object._kokkos_node_ids;
  _kokkos_element_side_ids = object._kokkos_element_side_ids;
}

void
BlockRestrictable::initializeKokkosBlockRestrictable()
{
  if (_moose_object->isParamValid("_kokkos_material"))
    return;

  const auto & blocks = blockRestricted() ? blockIDs() : meshBlockIDs();

  const auto * const kokkos_mesh = _blk_mesh->getKokkosMesh();

  std::vector<ContiguousElementID> element_ids;
  std::vector<ContiguousNodeID> node_ids;
  std::set<Moose::Kokkos::Pair<ContiguousElementID, unsigned int>> element_side_ids;

  for (const auto block : blocks)
  {
    element_ids.insert(element_ids.end(),
                       kokkos_mesh->getSubdomainContiguousElementIDRange(block).begin(),
                       kokkos_mesh->getSubdomainContiguousElementIDRange(block).end());
    node_ids.insert(node_ids.end(),
                    kokkos_mesh->getSubdomainContiguousNodeIDs(block).begin(),
                    kokkos_mesh->getSubdomainContiguousNodeIDs(block).end());

    for (const auto elem : _blk_mesh->getMesh().active_local_subdomain_elements_ptr_range(block))
    {
      const auto eid = kokkos_mesh->getContiguousElementID(elem);

      for (const auto side : make_range(elem->n_sides()))
        element_side_ids.insert({eid, side});
    }
  }

  _kokkos_element_ids = element_ids;

  if (_moose_object->isParamValid("unique_node_execute") &&
      !_moose_object->getParam<bool>("unique_node_execute"))
    _kokkos_node_ids = node_ids;
  else
    _kokkos_node_ids = std::set<ContiguousNodeID>(node_ids.begin(), node_ids.end());

  _kokkos_element_side_ids = element_side_ids;
}
