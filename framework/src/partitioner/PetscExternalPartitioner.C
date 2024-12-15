//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscExternalPartitioner.h"

#include "GeneratedMesh.h"
#include "MooseApp.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/petsc_solver_exception.h"

using namespace libMesh;

registerMooseObject("MooseApp", PetscExternalPartitioner);

#include <memory>

InputParameters
PetscExternalPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();

  MooseEnum partPackage("parmetis ptscotch chaco party hierarch", "parmetis", false);

  params.addParam<MooseEnum>("part_package",
                             partPackage,
                             "The external package is used for partitioning the mesh via PETSc");

  params.addParam<processor_id_type>(
      "num_cores_per_compute_node",
      1,
      "Number of cores per compute node for hierarchical partitioning");

  params.addParam<bool>("apply_element_weight",
                        false,
                        "Indicate if we are going to apply element weights to partitioners");

  params.addParam<bool>(
      "apply_side_weight", false, "Indicate if we are going to apply side weights to partitioners");

  params.addClassDescription(
      "Partition mesh using external packages via PETSc MatPartitioning interface");

  return params;
}

PetscExternalPartitioner::PetscExternalPartitioner(const InputParameters & params)
  : MoosePartitioner(params),
    _part_package(params.get<MooseEnum>("part_package")),
    _apply_element_weight(params.get<bool>("apply_element_weight")),
    _apply_side_weight(params.get<bool>("apply_side_weight")),
    _num_parts_per_compute_node(params.get<processor_id_type>("num_cores_per_compute_node"))
{
  if ((_apply_element_weight || _apply_side_weight) &&
      (_part_package == "chaco" || _part_package == "party"))
    mooseError(_part_package, " does not support weighted graph");
}

std::unique_ptr<Partitioner>
PetscExternalPartitioner::clone() const
{
  return _app.getFactory().clone(*this);
}

void
PetscExternalPartitioner::preLinearPartition(MeshBase & mesh)
{
  // Temporarily cache the old partition method
  auto old_partitioner = std::move(mesh.partitioner());
  // Create a linear partitioner
  mesh.partitioner() = std::make_unique<LinearPartitioner>();
  // Partition mesh
  mesh.partition(n_processors());
  // Restore the old partition
  mesh.partitioner() = std::move(old_partitioner);
}

void
PetscExternalPartitioner::partition(MeshBase & mesh, const unsigned int n_parts)
{
  // We want to use a parallel partitioner that requires a distributed graph
  // Simply calling a linear partitioner provides us the distributed graph
  // We should not do anything when using a distributed mesh since the mesh itself
  // is already distributed
  // When n_parts=1, we do not need to run any partitioner, instead, let libmesh
  // handle this
  if (mesh.is_replicated() && n_parts > 1)
    preLinearPartition(mesh);

  if (!isParamSetByUser("part_package") && (mesh.n_elem() / n_parts < 28) &&
      _part_package == "parmetis")
  {
    Moose::out
        << "Average number of elements per partition (" << mesh.n_elem() / n_parts
        << ") is less than 28. We are switching from ParMETIS to PTScotch for the partitioning."
        << std::endl;
    _part_package = "ptscotch";
  }

  Partitioner::partition(mesh, n_parts);
}

void
PetscExternalPartitioner::_do_partition(MeshBase & mesh, const unsigned int n_parts)
{
  initialize(mesh);

  dof_id_type num_edges, num_local_elems, local_elem_id, nj, side;
  std::vector<dof_id_type> side_weights;
  std::vector<dof_id_type> elem_weights;

  // Call libmesh to build the dual graph of mesh
  build_graph(mesh);
  num_local_elems = _dual_graph.size();

  elem_weights.clear();
  if (_apply_element_weight)
    elem_weights.resize(num_local_elems);

  num_edges = 0;
  // compute element weight
  for (dof_id_type k = 0; k < num_local_elems; k++)
  {
    num_edges += _dual_graph[k].size();
    if (_apply_element_weight)
    {
      // Get the original element
      mooseAssert(k < static_cast<dof_id_type>(_local_id_to_elem.size()),
                  "Local element id " << k << " is not smaller than " << _local_id_to_elem.size());
      auto elem = _local_id_to_elem[k];

      elem_weights[k] = computeElementWeight(*elem);
    }
  }

  side_weights.clear();
  // Edge weights represent the communication
  if (_apply_side_weight)
    side_weights.resize(num_edges);

  local_elem_id = 0;
  nj = 0;
  for (auto & row : _dual_graph)
  {
    mooseAssert(local_elem_id < static_cast<dof_id_type>(_local_id_to_elem.size()),
                "Local element id " << local_elem_id << " is not smaller than "
                                    << _local_id_to_elem.size());
    auto elem = _local_id_to_elem[local_elem_id];
    unsigned int n_neighbors = 0;

    side = 0;
    for (auto neighbor : elem->neighbor_ptr_range())
    {
      // Skip boundary sides since they do not connect to
      // anything.
      if (neighbor != nullptr && neighbor->active())
      {
        if (_apply_side_weight)
          side_weights[nj] = computeSideWeight(*elem, side);

        nj++;
        n_neighbors++;
      }

      side++;
    }
    if (n_neighbors != row.size())
      mooseError(
          "Cannot construct dual graph correctly since the number of neighbors is inconsistent");

    local_elem_id++;
  }

  std::vector<dof_id_type> partition;
  // Partition graph
  partitionGraph(comm(),
                 _dual_graph,
                 elem_weights,
                 side_weights,
                 n_parts,
                 _num_parts_per_compute_node,
                 _part_package,
                 partition);
  // Assign partition to mesh
  assign_partitioning(mesh, partition);
}

void
PetscExternalPartitioner::partitionGraph(const Parallel::Communicator & comm,
                                         const std::vector<std::vector<dof_id_type>> & graph,
                                         const std::vector<dof_id_type> & elem_weights,
                                         const std::vector<dof_id_type> & side_weights,
                                         const dof_id_type num_parts,
                                         const dof_id_type num_parts_per_compute_node,
                                         const std::string & part_package,
                                         std::vector<dof_id_type> & partition)
{
  Mat dual;
  PetscInt num_local_elems, num_elems, *xadj = nullptr, *adjncy = nullptr, i, *values = nullptr,
                                       *petsc_elem_weights = nullptr;
  const PetscInt * parts;
  MatPartitioning part;
  IS is;

  // Number of local elements
  num_elems = num_local_elems = graph.size();
  // Figure out the total of elements
  comm.sum(num_elems);

  LibmeshPetscCallA(comm.get(), PetscCalloc1(num_local_elems + 1, &xadj));

  num_local_elems = 0;
  xadj[0] = 0;
  for (auto & row : graph)
  {
    num_local_elems++;
    xadj[num_local_elems] = xadj[num_local_elems - 1] + row.size();
  }

  LibmeshPetscCallA(comm.get(), PetscCalloc1(xadj[num_local_elems], &adjncy));

  // Fill up adjacency
  i = 0;
  for (auto & row : graph)
    for (auto elem : row)
      adjncy[i++] = elem;

  // If there are no neighbors at all, no side weights should be proivded
  if (!i)
  {
    mooseAssert(!side_weights.size(),
                "No side weights should be provided since there are no neighbors at all");
  }

  // Copy over weights
  if (side_weights.size())
  {
    mooseAssert((PetscInt)side_weights.size() == i,
                "Side weight size " << side_weights.size()
                                    << " does not match with adjacency matrix size " << i);
    LibmeshPetscCallA(comm.get(), PetscCalloc1(side_weights.size(), &values));
    i = 0;
    for (auto weight : side_weights)
      values[i++] = weight;
  }

  LibmeshPetscCallA(
      comm.get(),
      MatCreateMPIAdj(comm.get(), num_local_elems, num_elems, xadj, adjncy, values, &dual));

  LibmeshPetscCallA(comm.get(), MatPartitioningCreate(comm.get(), &part));
#if !PETSC_VERSION_LESS_THAN(3, 12, 3)
  LibmeshPetscCallA(comm.get(), MatPartitioningSetUseEdgeWeights(part, PETSC_TRUE));
#endif
  LibmeshPetscCallA(comm.get(), MatPartitioningSetAdjacency(part, dual));

  if (!num_local_elems)
  {
    mooseAssert(!elem_weights.size(),
                "No element weights should be provided since there are no elements at all");
  }

  // Handle element weights
  if (elem_weights.size())
  {
    mooseAssert((PetscInt)elem_weights.size() == num_local_elems,
                "Element weight size " << elem_weights.size()
                                       << " does not match with the number of local elements"
                                       << num_local_elems);

    LibmeshPetscCallA(comm.get(), PetscCalloc1(elem_weights.size(), &petsc_elem_weights));
    i = 0;
    for (auto weight : elem_weights)
      petsc_elem_weights[i++] = weight;

    LibmeshPetscCallA(comm.get(), MatPartitioningSetVertexWeights(part, petsc_elem_weights));
  }

  LibmeshPetscCallA(comm.get(), MatPartitioningSetNParts(part, num_parts));
#if PETSC_VERSION_LESS_THAN(3, 9, 2)
  mooseAssert(part_package != "party", "PETSc-3.9.3 or higher is required for using party");
#endif
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  mooseAssert(part_package != "chaco", "PETSc-3.9.0 or higher is required for using chaco");
#endif
  LibmeshPetscCallA(comm.get(), MatPartitioningSetType(part, part_package.c_str()));
  if (part_package == "hierarch")
    LibmeshPetscCallA(comm.get(),
                      MatPartitioningHierarchicalSetNfineparts(part, num_parts_per_compute_node));

  LibmeshPetscCallA(comm.get(), MatPartitioningSetFromOptions(part));
  LibmeshPetscCallA(comm.get(), MatPartitioningApply(part, &is));

  LibmeshPetscCallA(comm.get(), ISGetIndices(is, &parts));

  partition.resize(num_local_elems);
  for (i = 0; i < num_local_elems; i++)
    partition[i] = parts[i];

  LibmeshPetscCallA(comm.get(), ISRestoreIndices(is, &parts));
  LibmeshPetscCallA(comm.get(), MatPartitioningDestroy(&part));
  LibmeshPetscCallA(comm.get(), MatDestroy(&dual));
  LibmeshPetscCallA(comm.get(), ISDestroy(&is));
}

dof_id_type
PetscExternalPartitioner::computeElementWeight(Elem & /*elem*/)
{
  return 1;
}

dof_id_type
PetscExternalPartitioner::computeSideWeight(Elem & /*elem*/, unsigned int /*side*/)
{
  return 1;
}
