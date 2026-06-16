//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELPartitioner.h"
#include "AbaqusUELMesh.h"
#include "PetscExternalPartitioner.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/int_range.h"
#include "libmesh/utility.h"

#include <set>

using namespace libMesh;

AbaqusUELPartitioner::AbaqusUELPartitioner(AbaqusUELMesh & uel_mesh)
  : Partitioner(), _uel_mesh(uel_mesh)
{
}

std::unique_ptr<Partitioner>
AbaqusUELPartitioner::clone() const
{
  return std::make_unique<AbaqusUELPartitioner>(*this);
}

void
AbaqusUELPartitioner::partition(MeshBase & mesh, const unsigned int n)
{
  // ParMETIS works on a distributed graph, so every rank must hold a chunk of the elements. A
  // replicated mesh has every element on every rank, so we first hand out a linear partitioning
  // to distribute the work before building the dual graph. This mirrors
  // PetscExternalPartitioner::preLinearPartition.
  if (mesh.is_replicated() && n > 1)
  {
    auto old_partitioner = std::move(mesh.partitioner());
    mesh.partitioner() = std::make_unique<LinearPartitioner>();
    mesh.partition(mesh.n_processors());
    mesh.partitioner() = std::move(old_partitioner);
  }

  Partitioner::partition(mesh, n);
}

void
AbaqusUELPartitioner::_do_partition(MeshBase & mesh, const unsigned int n)
{
  // Easy case: everything on processor 0.
  if (n == 1)
  {
    single_partition(mesh);
    return;
  }

  // Build the contiguous-by-processor global element index map (_global_index_by_pid_map) and
  // the per-processor active element counts (_n_active_elem_on_proc).
  _find_global_index_by_pid_map(mesh);

  const auto & elements = _uel_mesh.getElements();
  const auto & node_to_uel_map = _uel_mesh.getNodeToUELMap();

  const dof_id_type n_active_local_elem = mesh.n_active_local_elem();

  // Global index of this rank's first active element.
  dof_id_type first_local_elem = 0;
  for (const auto pid : make_range(mesh.processor_id()))
    first_local_elem += _n_active_elem_on_proc[pid];

  _dual_graph.clear();
  _dual_graph.resize(n_active_local_elem);
  _local_id_to_elem.clear();
  _local_id_to_elem.resize(n_active_local_elem);

  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    const dof_id_type global_index = libmesh_map_find(_global_index_by_pid_map, elem->id());
    const dof_id_type local_index = global_index - first_local_elem;
    libmesh_assert_less(local_index, n_active_local_elem);

    _local_id_to_elem[local_index] = const_cast<Elem *>(elem);

    // Connect this node-element to every other node-element that shares a UEL element with it.
    // The neighbors are stored in a set to deduplicate shared nodes and to keep a deterministic
    // ordering. The AbaqusUELRelationshipManager ghosts exactly these node-elements, so their
    // global indices are available in _global_index_by_pid_map.
    std::set<dof_id_type> neighbors;
    const auto it = node_to_uel_map.find(elem->id());
    if (it != node_to_uel_map.end())
      for (const auto uel_index : it->second)
        for (const auto node_index : elements[uel_index]._nodes)
          if (node_index != elem->id())
            neighbors.insert(libmesh_map_find(_global_index_by_pid_map, node_index));

    _dual_graph[local_index].assign(neighbors.begin(), neighbors.end());
  }

  // Partition the dual graph with ParMETIS via PETSc's MatPartitioning interface.
  std::vector<dof_id_type> partition;
  PetscExternalPartitioner::partitionGraph(mesh.comm(),
                                           _dual_graph,
                                           /* elem_weights = */ {},
                                           /* side_weights = */ {},
                                           n,
                                           /* num_parts_per_compute_node = */ 1,
                                           "parmetis",
                                           partition);

  assign_partitioning(mesh, partition);
}
