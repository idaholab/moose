//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscMatPartitioner.h"

#include "GeneratedMesh.h"
#include "MooseApp.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", PetscMatPartitioner);

#include <memory>

template <>
InputParameters
validParams<PetscMatPartitioner>()
{
  InputParameters params = validParams<MoosePartitioner>();

  MooseEnum partPackage("parmetis ptscotch chaco party hierarch", "ptscotch", false);

  params.addParam<MooseEnum>("part_package",
                             partPackage,
                             "The external package is used for partitioning the mesh via PETSc");

  params.addClassDescription(
      "Partition mesh using external packages via PETSc MatPartitioning interface");

  return params;
}

PetscMatPartitioner::PetscMatPartitioner(const InputParameters & params)
  : MoosePartitioner(params), _part_package(params.get<MooseEnum>("part_package"))
{
}

PetscMatPartitioner::~PetscMatPartitioner() {}

std::unique_ptr<Partitioner>
PetscMatPartitioner::clone() const
{
  return libmesh_make_unique<PetscMatPartitioner>(_pars);
}

void
PetscMatPartitioner::_do_partition(MeshBase & mesh, const unsigned int n_parts)
{
#ifdef LIBMESH_HAVE_PETSC
  // construct a dual graph
  Mat dual;
  PetscInt *i, *j, *values, nrows, nj, ncols;
  const PetscInt * parts;
  MatPartitioning part;
  IS is;

  i = j = values = 0;

  build_graph(mesh);
  nrows = _dual_graph.size();
  PetscCalloc1(nrows + 1, &i);
  nj = 0;
  for (PetscInt k = 0; k < nrows; k++)
    i[k + 1] = i[k] + _dual_graph[k].size();

  PetscCalloc1(i[nrows], &j);

  for (auto & row : _dual_graph)
    for (auto adj : row)
      j[nj++] = adj;

  ncols = 0;
  for (processor_id_type pid = 0; pid < mesh.n_processors(); pid++)
    ncols += _n_active_elem_on_proc[pid];

  MatCreateMPIAdj(mesh.comm().get(), nrows, ncols, i, j, values, &dual);
  MatPartitioningCreate(mesh.comm().get(), &part);
  MatPartitioningSetAdjacency(part, dual);
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
