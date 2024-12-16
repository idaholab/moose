//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PETScDMDAMesh.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/partitioner.h"
#include "libmesh/metis_csr_graph.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/remote_elem.h"
#include "libmesh/face_quad4.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/petsc_solver_exception.h"
#include "ExternalPetscSolverApp.h"
// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("ExternalPetscSolverApp", PETScDMDAMesh);

InputParameters
PETScDMDAMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();

  params.addClassDescription("Create a square mesh from PETSc DMDA.");

  // This mesh is always distributed
  params.set<MooseEnum>("parallel_type") = "DISTRIBUTED";

  return params;
}

PETScDMDAMesh::PETScDMDAMesh(const InputParameters & parameters) : MooseMesh(parameters)
{
  // Get TS from ExternalPetscSolverApp
  ExternalPetscSolverApp * petsc_app = dynamic_cast<ExternalPetscSolverApp *>(&_app);

  if (petsc_app && petsc_app->getPetscTS())
    // Retrieve mesh from TS
    LibmeshPetscCall(TSGetDM(petsc_app->getPetscTS(), &_dmda));
  else
    mooseError(" PETSc external solver TS does not exist or this is not a petsc external solver");
}

std::unique_ptr<MooseMesh>
PETScDMDAMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

inline dof_id_type
node_id_Quad4(const dof_id_type nx,
              const dof_id_type /*ny*/,
              const dof_id_type i,
              const dof_id_type j,
              const dof_id_type /*k*/)

{
  // Transform a grid coordinate (i, j) to its global node ID
  // This match what PETSc does
  return i + j * (nx + 1);
}

void
add_element_Quad4(DM da,
                  const dof_id_type nx,
                  const dof_id_type ny,
                  const dof_id_type i,
                  const dof_id_type j,
                  const dof_id_type elem_id,
                  const processor_id_type pid,
                  MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Mx: number of grid points in x direction for all processors
  // My: number of grid points in y direction for all processors
  // xp: number of processors in x direction
  // yp: number of processors in y direction
  PetscInt Mx, My, xp, yp;
  LibmeshPetscCallA(mesh.comm().get(),
                    DMDAGetInfo(da,
                                PETSC_IGNORE,
                                &Mx,
                                &My,
                                PETSC_IGNORE,
                                &xp,
                                &yp,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE));

  const PetscInt *lx, *ly;
  PetscInt *lxo, *lyo;
  // PETSc-3.8.x or older use PetscDataType
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  LibmeshPetscCallA(mesh.comm().get(), DMGetWorkArray(da, xp + yp + 2, PETSC_INT, &lxo));
#else
  // PETSc-3.9.x or newer use MPI_DataType
  LibmeshPetscCallA(mesh.comm().get(), DMGetWorkArray(da, xp + yp + 2, MPIU_INT, &lxo));
#endif

  // Gets the ranges of indices in the x, y and z direction that are owned by each process
  // Ranges here are different from what we have in Mat and Vec.
  // It means how many points each processor holds
  LibmeshPetscCallA(mesh.comm().get(), DMDAGetOwnershipRanges(da, &lx, &ly, NULL));
  lxo[0] = 0;
  for (PetscInt i = 0; i < xp; i++)
    lxo[i + 1] = lxo[i] + lx[i];

  lyo = lxo + xp + 1;
  lyo[0] = 0;
  for (PetscInt i = 0; i < yp; i++)
    lyo[i + 1] = lyo[i] + ly[i];

  // Try to calculate processor-grid coordinate (xpid, ypid)
  PetscInt xpid, ypid, xpidplus, ypidplus;
  // Finds integer in a sorted array of integers
  // Loc:  the location if found, otherwise -(slot+1)
  // where slot is the place the value would go
  LibmeshPetscCallA(mesh.comm().get(), PetscFindInt(i, xp + 1, lxo, &xpid));

  xpid = xpid < 0 ? -xpid - 1 - 1 : xpid;

  LibmeshPetscCallA(mesh.comm().get(), PetscFindInt(i + 1, xp + 1, lxo, &xpidplus));

  xpidplus = xpidplus < 0 ? -xpidplus - 1 - 1 : xpidplus;

  LibmeshPetscCallA(mesh.comm().get(), PetscFindInt(j, yp + 1, lyo, &ypid));

  ypid = ypid < 0 ? -ypid - 1 - 1 : ypid;

  LibmeshPetscCallA(mesh.comm().get(), PetscFindInt(j + 1, yp + 1, lyo, &ypidplus));

  ypidplus = ypidplus < 0 ? -ypidplus - 1 - 1 : ypidplus;
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  LibmeshPetscCallA(mesh.comm().get(), DMRestoreWorkArray(da, xp + yp + 2, PETSC_INT, &lxo));
#else
  LibmeshPetscCallA(mesh.comm().get(), DMRestoreWorkArray(da, xp + yp + 2, MPIU_INT, &lxo));
#endif

  // Bottom Left
  auto node0_ptr = mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0),
                                  node_id_Quad4(nx, 0, i, j, 0));
  node0_ptr->set_unique_id(node_id_Quad4(nx, 0, i, j, 0));
  node0_ptr->set_id() = node0_ptr->unique_id();
  // xpid + ypid * xp is the global processor ID
  node0_ptr->processor_id() = xpid + ypid * xp;

  // Bottom Right
  auto node1_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j) / ny, 0),
                     node_id_Quad4(nx, 0, i + 1, j, 0));
  node1_ptr->set_unique_id(node_id_Quad4(nx, 0, i + 1, j, 0));
  node1_ptr->set_id() = node1_ptr->unique_id();
  node1_ptr->processor_id() = xpidplus + ypid * xp;

  // Top Right
  auto node2_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id_Quad4(nx, 0, i + 1, j + 1, 0));
  node2_ptr->set_unique_id(node_id_Quad4(nx, 0, i + 1, j + 1, 0));
  node2_ptr->set_id() = node2_ptr->unique_id();
  node2_ptr->processor_id() = xpidplus + ypidplus * xp;

  // Top Left
  auto node3_ptr =
      mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id_Quad4(nx, 0, i, j + 1, 0));
  node3_ptr->set_unique_id(node_id_Quad4(nx, 0, i, j + 1, 0));
  node3_ptr->set_id() = node3_ptr->unique_id();
  node3_ptr->processor_id() = xpid + ypidplus * xp;

  // New an element and attach four nodes to it
  Elem * elem = new Quad4;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  // Make sure our unique_id doesn't overlap any nodes'
  elem->set_unique_id(elem_id + (nx + 1) * (ny + 1));
  elem = mesh.add_elem(elem);
  elem->set_node(0) = node0_ptr;
  elem->set_node(1) = node1_ptr;
  elem->set_node(2) = node2_ptr;
  elem->set_node(3) = node3_ptr;

  // Bottom
  if (j == 0)
    boundary_info.add_side(elem, 0, 0);

  // Right
  if (i == nx - 1)
    boundary_info.add_side(elem, 1, 1);

  // Top
  if (j == ny - 1)
    boundary_info.add_side(elem, 2, 2);

  // Left
  if (i == 0)
    boundary_info.add_side(elem, 3, 3);
}

void
set_boundary_names_Quad4(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "right";
  boundary_info.sideset_name(2) = "top";
  boundary_info.sideset_name(3) = "left";
}

void
get_indices_Quad4(const dof_id_type nx,
                  const dof_id_type /*ny*/,
                  const dof_id_type elem_id,
                  dof_id_type & i,
                  dof_id_type & j)
{
  i = elem_id % nx;
  j = (elem_id - i) / nx;
}

dof_id_type
elem_id_Quad4(const dof_id_type nx,
              const dof_id_type /*nx*/,
              const dof_id_type i,
              const dof_id_type j,
              const dof_id_type /*k*/)
{
  return (j * nx) + i;
}

inline void
get_neighbors_Quad4(const dof_id_type nx,
                    const dof_id_type ny,
                    const dof_id_type i,
                    const dof_id_type j,
                    std::vector<dof_id_type> & neighbors)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

  // Bottom
  if (j != 0)
    neighbors[0] = elem_id_Quad4(nx, 0, i, j - 1, 0);

  // Right
  if (i != nx - 1)
    neighbors[1] = elem_id_Quad4(nx, 0, i + 1, j, 0);

  // Top
  if (j != ny - 1)
    neighbors[2] = elem_id_Quad4(nx, 0, i, j + 1, 0);

  // Left
  if (i != 0)
    neighbors[3] = elem_id_Quad4(nx, 0, i - 1, j, 0);
}

void
get_ghost_neighbors_Quad4(const dof_id_type nx,
                          const dof_id_type ny,
                          const MeshBase & mesh,
                          std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.get_boundary_info();

  dof_id_type i, j;

  std::vector<dof_id_type> neighbors(4);

  for (auto elem_ptr : mesh.element_ptr_range())
  {
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
    {
      // No current neighbor
      if (!elem_ptr->neighbor_ptr(s))
      {
        // Not on a boundary
        if (!boundary_info.n_boundary_ids(elem_ptr, s))
        {
          auto elem_id = elem_ptr->id();

          get_indices_Quad4(nx, 0, elem_id, i, j);

          get_neighbors_Quad4(nx, ny, i, j, neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}

void
add_node_Qua4(dof_id_type nx,
              dof_id_type ny,
              dof_id_type i,
              dof_id_type j,
              processor_id_type pid,
              MeshBase & mesh)
{
  // Bottom Left
  auto node0_ptr = mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0),
                                  node_id_Quad4(nx, 0, i, j, 0));
  node0_ptr->set_unique_id(node_id_Quad4(nx, 0, i, j, 0));
  node0_ptr->processor_id() = pid;
}

void
build_cube_Quad4(UnstructuredMesh & mesh, DM da)
{
  const auto pid = mesh.comm().rank();

  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  // xs: start grid point (not element) index on local in x direction
  // ys: start grid point index on local in y direction
  // xm: number of grid points owned by the local processor in x direction
  // ym: number of grid points owned by the local processor in y direction
  // Mx: number of grid points on all processors in x direction
  // My: number of grid points on all processors in y direction
  // xp: number of processor cores in x direction
  // yp: number of processor cores in y direction
  PetscInt xs, ys, xm, ym, Mx, My, xp, yp;

  /* Get local grid boundaries */
  LibmeshPetscCallA(mesh.comm().get(),
                    DMDAGetCorners(da, &xs, &ys, PETSC_IGNORE, &xm, &ym, PETSC_IGNORE));
  LibmeshPetscCallA(mesh.comm().get(),
                    DMDAGetInfo(da,
                                PETSC_IGNORE,
                                &Mx,
                                &My,
                                PETSC_IGNORE,
                                &xp,
                                &yp,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE,
                                PETSC_IGNORE));

  for (PetscInt j = ys; j < ys + ym; j++)
    for (PetscInt i = xs; i < xs + xm; i++)
    {
      // We loop over grid points, but we are
      // here building elements. So that we just
      // simply skip the first x and y points since the
      // number of grid ponts is one more than
      // the number of grid elements
      if (!i || !j)
        continue;

      dof_id_type ele_id = (i - 1) + (j - 1) * (Mx - 1);

      add_element_Quad4(da, Mx - 1, My - 1, i - 1, j - 1, ele_id, pid, mesh);
    }

  // If there is no element at the given processor
  // We need to manually add all mesh nodes
  if ((ys == 0 && ym == 1) || (xs == 0 && xm == 1))
    for (PetscInt j = ys; j < ys + ym; j++)
      for (PetscInt i = xs; i < xs + xm; i++)
        add_node_Qua4(Mx, My, i, j, pid, mesh);

  // Need to link up the local elements before we can know what's missing
  mesh.find_neighbors();

  mesh.find_neighbors(true);

  // Set RemoteElem neighbors
  for (auto & elem_ptr : mesh.element_ptr_range())
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
      if (!elem_ptr->neighbor_ptr(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
        elem_ptr->set_neighbor(s, const_cast<RemoteElem *>(remote_elem));

  set_boundary_names_Quad4(boundary_info);
  // Already partitioned!
  mesh.skip_partitioning(true);

  // We've already set our own unique_id values; now make sure that
  // future mesh modifications use subsequent values.
  mesh.set_next_unique_id(Mx * My + (Mx - 1) * (My - 1));

  // No need to renumber or find neighbors - done did it.
  mesh.allow_renumbering(false);
  mesh.allow_find_neighbors(false);
  mesh.prepare_for_use();
  mesh.allow_find_neighbors(true);
}

void
PETScDMDAMesh::buildMesh()
{
  // Reference to the libmesh mesh
  MeshBase & mesh = getMesh();

  build_cube_Quad4(dynamic_cast<UnstructuredMesh &>(mesh), _dmda);
}
