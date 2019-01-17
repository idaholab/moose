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

#include "ExternalPetscSolverApp.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("ExternalPetscSolverApp", PETScDMDAMesh);

template <>
InputParameters
validParams<PETScDMDAMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  MooseEnum elem_types("EDGE2  QUAD4  HEX8"); // no default

  MooseEnum dims("1=1 2 3", "2");
  params.addRequiredParam<MooseEnum>(
      "dim", dims, "The dimension of the mesh to be generated"); // Make this parameter required

  params.addParam<dof_id_type>("nx", 11, "Number of elements in the X direction");
  params.addParam<dof_id_type>("ny", 11, "Number of elements in the Y direction");
  params.addParam<dof_id_type>("nz", 11, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "The type of element from libMesh to "
                             "generate (default: linear element for "
                             "requested dimension)");

  params.addParamNamesToGroup("dim", "Main");

  params.addClassDescription(
      "Create a line, square, or cube mesh with uniformly spaced memsh using PETSc DMDA.");

  // This mesh is always distributed
  params.set<MooseEnum>("parallel_type") = "DISTRIBUTED";

  return params;
}

PETScDMDAMesh::PETScDMDAMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<dof_id_type>("nx")),
    _ny(getParam<dof_id_type>("ny")),
    _nz(getParam<dof_id_type>("nz")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax"))
{
  // All generated meshes are regular orthogonal meshes
  _regular_orthogonal_mesh = true;

  // We support 2D mesh only at this point. If you need 3D or 1D
  // Please contact MOOSE developers
  if (_dim != 2)
    mooseError("Support 2 dimensional mesh only");

#if LIBMESH_HAVE_PETSC
  // If we are going to couple PETSc external solver,
  // take a DA from PETSc
  ExternalPetscSolverApp * petsc_app = dynamic_cast<ExternalPetscSolverApp *>(&_app);
  if (petsc_app)
  {
    TS & ts = petsc_app->getExternalPETScTS();
    // Retrieve mesh from TS
    TSGetDM(ts, &_dmda);
    _need_to_destroy_dmda = false;
  }
  // This mesh object can be used independently for any MOOSE apps.
  // We create one from scratch
  else
  {
    DMDACreate2d(_communicator.get(),
                 DM_BOUNDARY_NONE,
                 DM_BOUNDARY_NONE,
                 DMDA_STENCIL_BOX,
                 _nx,
                 _ny,
                 PETSC_DECIDE,
                 PETSC_DECIDE,
                 1,
                 1,
                 NULL,
                 NULL,
                 &_dmda);
    // Let DMDA take PETSc options
    DMSetFromOptions(_dmda);
    // Finalize mesh setup
    DMSetUp(_dmda);
    _need_to_destroy_dmda = true;
  }
#else
  mooseError("You need PETSc installed to use PETScDMDAMesh");
#endif
}

Real
PETScDMDAMesh::getMinInDimension(unsigned int component) const
{
  switch (component)
  {
    case 0:
      return _xmin;
    case 1:
      return _dim > 1 ? _ymin : 0;
    case 2:
      return _dim > 2 ? _zmin : 0;
    default:
      mooseError("Invalid component");
  }
}

Real
PETScDMDAMesh::getMaxInDimension(unsigned int component) const
{
  switch (component)
  {
    case 0:
      return _xmax;
    case 1:
      return _dim > 1 ? _ymax : 0;
    case 2:
      return _dim > 2 ? _zmax : 0;
    default:
      mooseError("Invalid component");
  }
}

std::unique_ptr<MooseMesh>
PETScDMDAMesh::safeClone() const
{
  return libmesh_make_unique<PETScDMDAMesh>(*this);
}

inline dof_id_type
node_id_Quad4(const ElemType /*type*/,
              const dof_id_type nx,
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
                  const ElemType type,
                  MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

#if LIBMESH_HAVE_PETSC
  // Mx: number of grid points in x direction for all processors
  // My: number of grid points in y direction for all processors
  // xp: number of processors in x direction
  // yp: number of processors in y direction
  PetscInt Mx, My, xp, yp;
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
              PETSC_IGNORE);

  const PetscInt *lx, *ly;
  PetscInt *lxo, *lyo;
  // PETSc-3.8.x or older use PetscDataType
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  DMGetWorkArray(da, xp + yp + 2, PETSC_INT, &lxo);
#else
  // PETSc-3.9.x or newer use MPI_DataType
  DMGetWorkArray(da, xp + yp + 2, MPIU_INT, &lxo);
#endif
  // Gets the ranges of indices in the x, y and z direction that are owned by each process
  // Ranges here are different from what we have in Mat and Vec.
  // It means how many points each processor holds
  DMDAGetOwnershipRanges(da, &lx, &ly, NULL);
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
  PetscFindInt(i, xp + 1, lxo, &xpid);

  xpid = xpid < 0 ? -xpid - 1 - 1 : xpid;

  PetscFindInt(i + 1, xp + 1, lxo, &xpidplus);

  xpidplus = xpidplus < 0 ? -xpidplus - 1 - 1 : xpidplus;

  PetscFindInt(j, yp + 1, lyo, &ypid);

  ypid = ypid < 0 ? -ypid - 1 - 1 : ypid;

  PetscFindInt(j + 1, yp + 1, lyo, &ypidplus);

  ypidplus = ypidplus < 0 ? -ypidplus - 1 - 1 : ypidplus;
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  DMRestoreWorkArray(da, xp + yp + 2, PETSC_INT, &lxo);
#else
  DMRestoreWorkArray(da, xp + yp + 2, MPIU_INT, &lxo);
#endif
#endif
  // Bottom Left
  auto node0_ptr = mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0),
                                  node_id_Quad4(type, nx, 0, i, j, 0));
  node0_ptr->set_unique_id() = node_id_Quad4(type, nx, 0, i, j, 0);
  node0_ptr->set_id() = node0_ptr->unique_id();
  // xpid + ypid * xp is the global processor ID
  node0_ptr->processor_id() = xpid + ypid * xp;

  // Bottom Right
  auto node1_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j) / ny, 0),
                     node_id_Quad4(type, nx, 0, i + 1, j, 0));
  node1_ptr->set_unique_id() = node_id_Quad4(type, nx, 0, i + 1, j, 0);
  node1_ptr->set_id() = node1_ptr->unique_id();
  node1_ptr->processor_id() = xpidplus + ypid * xp;

  // Top Right
  auto node2_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id_Quad4(type, nx, 0, i + 1, j + 1, 0));
  node2_ptr->set_unique_id() = node_id_Quad4(type, nx, 0, i + 1, j + 1, 0);
  node2_ptr->set_id() = node2_ptr->unique_id();
  node2_ptr->processor_id() = xpidplus + ypidplus * xp;

  // Top Left
  auto node3_ptr =
      mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id_Quad4(type, nx, 0, i, j + 1, 0));
  node3_ptr->set_unique_id() = node_id_Quad4(type, nx, 0, i, j + 1, 0);
  node3_ptr->set_id() = node3_ptr->unique_id();
  node3_ptr->processor_id() = xpid + ypidplus * xp;

  // New an element and attach four nodes to it
  Elem * elem = new Quad4;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  elem->set_unique_id() = elem_id;
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
              ElemType type,
              MeshBase & mesh)
{
  // Bottom Left
  auto node0_ptr = mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0),
                                  node_id_Quad4(type, nx, 0, i, j, 0));
  node0_ptr->set_unique_id() = node_id_Quad4(type, nx, 0, i, j, 0);
  node0_ptr->processor_id() = pid;
}

#if LIBMESH_HAVE_PETSC
void
build_cube_Quad4(UnstructuredMesh & mesh, DM da, const ElemType type)
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
  DMDAGetCorners(da, &xs, &ys, PETSC_IGNORE, &xm, &ym, PETSC_IGNORE);
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
              PETSC_IGNORE);

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

      add_element_Quad4(da, Mx - 1, My - 1, i - 1, j - 1, ele_id, pid, type, mesh);
    }

  // If there is no element at the given processor
  // We need to manually add all mesh nodes
  if ((ys == 0 && ym == 1) || (xs == 0 && xm == 1))
    for (PetscInt j = ys; j < ys + ym; j++)
      for (PetscInt i = xs; i < xs + xm; i++)
        add_node_Qua4(Mx, My, i, j, pid, type, mesh);

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

  // No need to renumber or find neighbors - done did it.
  // Avoid deprecation message/error by _also_ setting
  // allow_renumbering(false). This is a bit silly, but we want to
  // catch cases where people are purely using the old "skip"
  // interface and not the new flag setting one.
  mesh.allow_renumbering(false);
  mesh.prepare_for_use(/*skip_renumber (ignored!) = */ false,
                       /*skip_find_neighbors = */ true);
}
#endif

void
PETScDMDAMesh::buildMesh()
{
  // Reference to the libmesh mesh
  MeshBase & mesh = getMesh();

  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
      case 2:
        elem_type_enum = "QUAD4";
        break;

      default:
        mooseError("Does not support dimension ", _dim, "yet");
    }
  }

  _elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  mesh.set_mesh_dimension(_dim);
  mesh.set_spatial_dimension(_dim);

  // Switching on MooseEnum
  switch (_dim)
  {
#if LIBMESH_HAVE_PETSC
    case 2:
      build_cube_Quad4(dynamic_cast<UnstructuredMesh &>(getMesh()), _dmda, _elem_type);
      break;
#endif
    default:
      mooseError("Does not support dimension ", _dim, "yet");
  }
}
