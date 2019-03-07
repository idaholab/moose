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

registerMooseObject("MooseApp", PetscExternalPartitioner);

#include <memory>

template <>
InputParameters
validParams<PetscExternalPartitioner>()
{
  InputParameters params = validParams<MoosePartitioner>();

  MooseEnum partPackage("parmetis ptscotch chaco party hierarch", "parmetis", false);

  params.addParam<MooseEnum>("part_package",
                             partPackage,
                             "The external package is used for partitioning the mesh via PETSc");

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
    _apply_side_weight(params.get<bool>("apply_side_weight"))
{
  if ((_apply_element_weight || _apply_side_weight) &&
      (_part_package == "chaco" || _part_package == "party"))
    mooseError(_part_package, " does not support weighted graph");
}

std::unique_ptr<Partitioner>
PetscExternalPartitioner::clone() const
{
  return libmesh_make_unique<PetscExternalPartitioner>(_pars);
}

void
PetscExternalPartitioner::preLinearPartition(MeshBase & mesh)
{
  // Temporarily cache the old partition method
  auto old_partitioner = std::move(mesh.partitioner());
  // Create a linear partitioner
  mesh.partitioner().reset(new LinearPartitioner);
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
  // We shold not do anything when using a distributed mesh since the mesh itself
  // is already distributed
  // When n_parts=1, we do not need to run any partitioner, instead, let libmesh
  // handle this
  if (mesh.is_replicated() && n_parts > 1)
    preLinearPartition(mesh);

  Partitioner::partition(mesh, n_parts);
}

void
PetscExternalPartitioner::_do_partition(MeshBase & mesh, const unsigned int n_parts)
{
#ifdef LIBMESH_HAVE_PETSC
  // construct a dual graph
  Mat dual;
  PetscInt *i, *j, *values, *elem_weights, nrows, nj, ncols, local_elem_id;
  const PetscInt * parts;
  MatPartitioning part;
  IS is;

  i = 0;
  j = 0;
  values = 0;
  elem_weights = 0;

  build_graph(mesh);
  nrows = _dual_graph.size();
  PetscCalloc1(nrows + 1, &i);
  if (_apply_element_weight)
    PetscCalloc1(nrows + 1, &elem_weights);

  // Set graph offsets and
  // compute element weight
  for (PetscInt k = 0; k < nrows; k++)
  {
    i[k + 1] = i[k] + _dual_graph[k].size();

    if (_apply_element_weight)
    {
      // Get the original element
      auto & elem = _local_id_to_elem[k];

      elem_weights[k] = computeElementWeight(*elem);
    }
  }

  // Graph adjacency
  PetscCalloc1(i[nrows], &j);

  // Edge weights represent the communication
  if (_apply_side_weight)
    PetscCalloc1(i[nrows], &values);

  local_elem_id = 0;
  nj = 0;
  for (auto & row : _dual_graph)
  {
    auto & elem = _local_id_to_elem[local_elem_id];
    unsigned int n_neighbors = 0;

    for (auto neighbor : elem->neighbor_ptr_range())
    {
      // Skip boundary sides since they do not connect to
      // anything.
      if (neighbor != nullptr && neighbor->active())
      {
        j[nj] = row[n_neighbors++];

        if (_apply_side_weight)
        {
          auto side = elem->which_neighbor_am_i(neighbor);
          values[nj] = computeSideWeight(*elem, side);
        }

        nj++;
      }
    }
    if (n_neighbors != row.size())
      mooseError(
          "Cannot construct dual graph correctly since the number of neighbors is inconsistent");

    local_elem_id++;
  }

  ncols = 0;
  for (processor_id_type pid = 0; pid < mesh.n_processors(); pid++)
    ncols += _n_active_elem_on_proc[pid];

  MatCreateMPIAdj(mesh.comm().get(), nrows, ncols, i, j, values, &dual);
  MatPartitioningCreate(mesh.comm().get(), &part);
  MatPartitioningSetAdjacency(part, dual);

  if (_apply_element_weight)
    MatPartitioningSetVertexWeights(part, elem_weights);

  MatPartitioningSetNParts(part, n_parts);
#if PETSC_VERSION_LESS_THAN(3, 9, 2)
  if (_part_package == "party")
    mooseError("PETSc-3.9.3 or higher is required for using party");
#endif
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  if (_part_package == "chaco")
    mooseError("PETSc-3.9.0 or higher is required for using chaco");
#endif
  MatPartitioningSetType(part, _part_package.c_str());
  MatPartitioningSetFromOptions(part);
  MatPartitioningApply(part, &is);

  ISGetIndices(is, &parts);

  std::vector<dof_id_type> libmesh_parts;
  std::copy(parts, parts + nrows, std::back_inserter(libmesh_parts));

  ISRestoreIndices(is, &parts);

  assign_partitioning(mesh, libmesh_parts);

  ISRestoreIndices(is, &parts);

  MatPartitioningDestroy(&part);
  ISDestroy(&is);
  MatDestroy(&dual);
#else
  mooseError("Petsc is required for this partitioner");
#endif
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
