//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedRectilinearMeshGenerator.h"
#include "PetscExternalPartitioner.h"

#include "SerializerGuard.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/remote_elem.h"
#include "libmesh/face_quad4.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/partitioner.h"

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseApp", DistributedRectilinearMeshGenerator);

InputParameters
DistributedRectilinearMeshGenerator::validParams()
{
  InputParameters params = PetscExternalPartitioner::validParams();
  params += MeshGenerator::validParams();

  params.addParam<bool>("verbose", false, "Turn on verbose printing for the mesh generation");

  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>(
      "dim", dims, "The dimension of the mesh to be generated"); // Make this parameter required

  params.addParam<dof_id_type>("nx", 1, "Number of elements in the X direction");
  params.addParam<dof_id_type>("ny", 1, "Number of elements in the Y direction");
  params.addParam<dof_id_type>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");

  MooseEnum elem_types(
      "EDGE EDGE2 EDGE3 EDGE4 QUAD QUAD4 QUAD8 QUAD9 TRI3 TRI6 HEX HEX8 HEX20 HEX27 TET4 TET10 "
      "PRISM6 PRISM15 PRISM18 PYRAMID5 PYRAMID13 PYRAMID14"); // no default
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "The type of element from libMesh to "
                             "generate (default: linear element for "
                             "requested dimension)");

  params.addRangeCheckedParam<Real>(
      "bias_x",
      1.,
      "bias_x>=0.5 & bias_x<=2",
      "The amount by which to grow (or shrink) the cells in the x-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_y",
      1.,
      "bias_y>=0.5 & bias_y<=2",
      "The amount by which to grow (or shrink) the cells in the y-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_z",
      1.,
      "bias_z>=0.5 & bias_z<=2",
      "The amount by which to grow (or shrink) the cells in the z-direction.");

  params.addParamNamesToGroup("dim", "Main");

  params.addClassDescription(
      "Create a line, square, or cube mesh with uniformly spaced or biased elements.");

  return params;
}

DistributedRectilinearMeshGenerator::DistributedRectilinearMeshGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _verbose(getParam<bool>("verbose")),
    _dim(getParam<MooseEnum>("dim")),
    _nx(declareMeshProperty("num_elements_x", getParam<unsigned int>("nx"))),
    _ny(declareMeshProperty("num_elements_y", getParam<unsigned int>("ny"))),
    _nz(declareMeshProperty("num_elements_z", getParam<unsigned int>("nz"))),
    _xmin(declareMeshProperty("xmin", getParam<Real>("xmin"))),
    _xmax(declareMeshProperty("xmax", getParam<Real>("xmax"))),
    _ymin(declareMeshProperty("ymin", getParam<Real>("ymin"))),
    _ymax(declareMeshProperty("ymax", getParam<Real>("ymax"))),
    _zmin(declareMeshProperty("zmin", getParam<Real>("zmin"))),
    _zmax(declareMeshProperty("zmax", getParam<Real>("zmax"))),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z")),
    _part_package(getParam<MooseEnum>("part_package")),
    _num_parts_per_compute_node(getParam<dof_id_type>("num_cores_per_compute_node"))
{
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::num_neighbors<Edge2>(const dof_id_type nx,
                                                          const dof_id_type /*ny*/,
                                                          const dof_id_type /*nz*/,
                                                          const dof_id_type i,
                                                          const dof_id_type /*j*/,
                                                          const dof_id_type /*k*/)
{
  // The ends only have one neighbor
  if (i == 0 || i == nx - 1)
    return 1;

  return 2;
}

template <typename T>
inline void
get_neighbors(const dof_id_type /*nx*/,
              const dof_id_type /*ny*/,
              const dof_id_type /*nz*/,
              const dof_id_type /*i*/,
              const dof_id_type /*j*/,
              const dof_id_type /*k*/,
              std::vector<dof_id_type> & /*neighbors*/,
              const bool /*corner*/)
{
  mooseError("get_neighbors not implemented for this element type in DistributedGeneratedMesh");
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_neighbors<Edge2>(const dof_id_type nx,
                                                          const dof_id_type /*ny*/,
                                                          const dof_id_type /*nz*/,
                                                          const dof_id_type i,
                                                          const dof_id_type /*j*/,
                                                          const dof_id_type /*k*/,
                                                          std::vector<dof_id_type> & neighbors,
                                                          const bool /*corner*/)

{
  neighbors[0] = i - 1;
  neighbors[1] = i + 1;

  // First element doesn't have a left neighbor
  if (i == 0)
    neighbors[0] = Elem::invalid_id;

  // Last element doesn't have a right neighbor
  if (i == nx - 1)
    neighbors[1] = Elem::invalid_id;
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_indices<Edge2>(const dof_id_type /*nx*/,
                                                        const dof_id_type /*ny*/,
                                                        const dof_id_type elem_id,
                                                        dof_id_type & i,
                                                        dof_id_type & /*j*/,
                                                        dof_id_type & /*k*/)
{
  i = elem_id;
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_ghost_neighbors<Edge2>(const dof_id_type nx,
                                                                const dof_id_type /*ny*/,
                                                                const dof_id_type /*nz*/,
                                                                const MeshBase & mesh,
                                                                std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.get_boundary_info();

  std::vector<dof_id_type> neighbors(2);

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
          get_neighbors<Edge2>(nx, 0, 0, elem_ptr->id(), 0, 0, neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::elem_id<Edge2>(const dof_id_type /*nx*/,
                                                    const dof_id_type /*ny*/,
                                                    const dof_id_type i,
                                                    const dof_id_type /*j*/,
                                                    const dof_id_type /*k*/)
{
  return i;
}

template <>
void
DistributedRectilinearMeshGenerator::add_element<Edge2>(const dof_id_type nx,
                                                        const dof_id_type /*ny*/,
                                                        const dof_id_type /*nz*/,
                                                        const dof_id_type /*i*/,
                                                        const dof_id_type /*j*/,
                                                        const dof_id_type /*k*/,
                                                        const dof_id_type elem_id,
                                                        const processor_id_type pid,
                                                        const ElemType /*type*/,
                                                        MeshBase & mesh,
                                                        bool verbose)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (verbose)
    Moose::out << "Adding elem: " << elem_id << " pid: " << pid << std::endl;

  auto node_offset = elem_id;

  auto node0_ptr = mesh.add_point(Point(static_cast<Real>(node_offset) / nx, 0, 0), node_offset);
  node0_ptr->set_unique_id() = node_offset;
  node0_ptr->processor_id() = pid;

  auto node1_ptr =
      mesh.add_point(Point(static_cast<Real>(node_offset + 1) / nx, 0, 0), node_offset + 1);
  node1_ptr->set_unique_id() = node_offset + 1;
  node1_ptr->processor_id() = pid;

  if (verbose)
    Moose::out << "Adding elem: " << elem_id << std::endl;

  Elem * elem = new Edge2;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  elem->set_unique_id() = elem_id;
  elem = mesh.add_elem(elem);
  elem->set_node(0) = node0_ptr;
  elem->set_node(1) = node1_ptr;

  if (elem_id == 0)
    boundary_info.add_side(elem, 0, 0);

  if (elem_id == nx - 1)
    boundary_info.add_side(elem, 1, 1);
}

template <>
void
DistributedRectilinearMeshGenerator::set_boundary_names<Edge2>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "left";
  boundary_info.sideset_name(1) = "right";
}

template <>
void
DistributedRectilinearMeshGenerator::scale_nodal_positions<Edge2>(dof_id_type /*nx*/,
                                                                  dof_id_type /*ny*/,
                                                                  dof_id_type /*nz*/,
                                                                  Real xmin,
                                                                  Real xmax,
                                                                  Real /*ymin*/,
                                                                  Real /*ymax*/,
                                                                  Real /*zmin*/,
                                                                  Real /*zmax*/,
                                                                  MeshBase & mesh)
{
  for (auto & node_ptr : mesh.node_ptr_range())
    (*node_ptr)(0) = (*node_ptr)(0) * (xmax - xmin) + xmin;
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::num_neighbors<Quad4>(const dof_id_type nx,
                                                          const dof_id_type ny,
                                                          const dof_id_type /*nz*/,
                                                          const dof_id_type i,
                                                          const dof_id_type j,
                                                          const dof_id_type /*k*/)
{
  dof_id_type n = 4;

  if (i == 0)
    n--;

  if (i == nx - 1)
    n--;

  if (j == 0)
    n--;

  if (j == ny - 1)
    n--;

  return n;
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::elem_id<Quad4>(const dof_id_type nx,
                                                    const dof_id_type /*nx*/,
                                                    const dof_id_type i,
                                                    const dof_id_type j,
                                                    const dof_id_type /*k*/)
{
  return (j * nx) + i;
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_neighbors<Quad4>(const dof_id_type nx,
                                                          const dof_id_type ny,
                                                          const dof_id_type /*nz*/,
                                                          const dof_id_type i,
                                                          const dof_id_type j,
                                                          const dof_id_type /*k*/,
                                                          std::vector<dof_id_type> & neighbors,
                                                          const bool corner)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

  if (corner)
  {
    int nnb = 0;
    for (int ii = -1; ii <= 1; ii++)
      for (int jj = -1; jj <= 1; jj++)
        if ((i + ii >= 0) && (i + ii < nx) && (j + jj >= 0) && (j + jj < ny))
          neighbors[nnb++] = elem_id<Quad4>(nx, 0, i + ii, j + jj, 0);

    return;
  }
  // Bottom
  if (j != 0)
    neighbors[0] = elem_id<Quad4>(nx, 0, i, j - 1, 0);

  // Right
  if (i != nx - 1)
    neighbors[1] = elem_id<Quad4>(nx, 0, i + 1, j, 0);

  // Top
  if (j != ny - 1)
    neighbors[2] = elem_id<Quad4>(nx, 0, i, j + 1, 0);

  // Left
  if (i != 0)
    neighbors[3] = elem_id<Quad4>(nx, 0, i - 1, j, 0);
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_indices<Quad4>(const dof_id_type nx,
                                                        const dof_id_type /*ny*/,
                                                        const dof_id_type elem_id,
                                                        dof_id_type & i,
                                                        dof_id_type & j,
                                                        dof_id_type & /*k*/)
{
  i = elem_id % nx;
  j = (elem_id - i) / nx;
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_ghost_neighbors<Quad4>(const dof_id_type nx,
                                                                const dof_id_type ny,
                                                                const dof_id_type /*nz*/,
                                                                const MeshBase & mesh,
                                                                std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.get_boundary_info();

  dof_id_type i, j, k;

  std::vector<dof_id_type> neighbors(9);

  for (auto elem_ptr : mesh.element_ptr_range())
  {
    auto elem_id = elem_ptr->id();

    get_indices<Quad4>(nx, 0, elem_id, i, j, k);

    get_neighbors<Quad4>(nx, ny, 0, i, j, 0, neighbors, true);

    for (auto neighbor : neighbors)
      if (neighbor != Elem::invalid_id && !mesh.query_elem_ptr(neighbor))
        ghost_elems.insert(neighbor);
  }
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::node_id<Quad4>(const ElemType /*type*/,
                                                    const dof_id_type nx,
                                                    const dof_id_type /*ny*/,
                                                    const dof_id_type i,
                                                    const dof_id_type j,
                                                    const dof_id_type /*k*/)

{
  return i + j * (nx + 1);
}

template <>
void
DistributedRectilinearMeshGenerator::add_element<Quad4>(const dof_id_type nx,
                                                        const dof_id_type ny,
                                                        const dof_id_type /*nz*/,
                                                        const dof_id_type i,
                                                        const dof_id_type j,
                                                        const dof_id_type /*k*/,
                                                        const dof_id_type elem_id,
                                                        const processor_id_type pid,
                                                        const ElemType type,
                                                        MeshBase & mesh,
                                                        bool verbose)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (verbose)
    Moose::out << "Adding elem: " << elem_id << " pid: " << pid << std::endl;

  // Bottom Left
  auto node0_ptr = mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0),
                                  node_id<Quad4>(type, nx, 0, i, j, 0));
  node0_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i, j, 0);
  node0_ptr->processor_id() = pid;

  // Bottom Right
  auto node1_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j) / ny, 0),
                     node_id<Quad4>(type, nx, 0, i + 1, j, 0));
  node1_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i + 1, j, 0);
  node1_ptr->processor_id() = pid;

  // Top Right
  auto node2_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id<Quad4>(type, nx, 0, i + 1, j + 1, 0));
  node2_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i + 1, j + 1, 0);
  node2_ptr->processor_id() = pid;

  // Top Left
  auto node3_ptr =
      mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id<Quad4>(type, nx, 0, i, j + 1, 0));
  node3_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i, j + 1, 0);
  node3_ptr->processor_id() = pid;

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

template <>
void
DistributedRectilinearMeshGenerator::set_boundary_names<Quad4>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "right";
  boundary_info.sideset_name(2) = "top";
  boundary_info.sideset_name(3) = "left";
}

template <>
void
DistributedRectilinearMeshGenerator::scale_nodal_positions<Quad4>(dof_id_type /*nx*/,
                                                                  dof_id_type /*ny*/,
                                                                  dof_id_type /*nz*/,
                                                                  Real xmin,
                                                                  Real xmax,
                                                                  Real ymin,
                                                                  Real ymax,
                                                                  Real /*zmin*/,
                                                                  Real /*zmax*/,
                                                                  MeshBase & mesh)
{
  for (auto & node_ptr : mesh.node_ptr_range())
  {
    (*node_ptr)(0) = (*node_ptr)(0) * (xmax - xmin) + xmin;
    (*node_ptr)(1) = (*node_ptr)(1) * (ymax - ymin) + ymin;
  }
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::elem_id<Hex8>(const dof_id_type nx,
                                                   const dof_id_type ny,
                                                   const dof_id_type i,
                                                   const dof_id_type j,
                                                   const dof_id_type k)
{
  return i + (j * nx) + (k * nx * ny);
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::num_neighbors<Hex8>(const dof_id_type nx,
                                                         const dof_id_type ny,
                                                         const dof_id_type nz,
                                                         const dof_id_type i,
                                                         const dof_id_type j,
                                                         const dof_id_type k)
{
  dof_id_type n = 6;

  if (i == 0)
    n--;

  if (i == nx - 1)
    n--;

  if (j == 0)
    n--;

  if (j == ny - 1)
    n--;

  if (k == 0)
    n--;

  if (k == nz - 1)
    n--;

  return n;
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_neighbors<Hex8>(const dof_id_type nx,
                                                         const dof_id_type ny,
                                                         const dof_id_type nz,
                                                         const dof_id_type i,
                                                         const dof_id_type j,
                                                         const dof_id_type k,
                                                         std::vector<dof_id_type> & neighbors,
                                                         const bool corner)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

  if (corner)
  {
    int nnb = 0;
    for (int ii = -1; ii <= 1; ii++)
      for (int jj = -1; jj <= 1; jj++)
        for (int kk = -1; kk <= 1; kk++)
          if ((i + ii >= 0) && (i + ii < nx) && (j + jj >= 0) && (j + jj < ny) && (k + kk >= 0) &&
              (k + kk < nz))
            neighbors[nnb++] = elem_id<Hex8>(nx, ny, i + ii, j + jj, k + kk);

    return;
  }

  // Back
  if (k != 0)
    neighbors[0] = elem_id<Hex8>(nx, ny, i, j, k - 1);

  // Bottom
  if (j != 0)
    neighbors[1] = elem_id<Hex8>(nx, ny, i, j - 1, k);

  // Right
  if (i != nx - 1)
    neighbors[2] = elem_id<Hex8>(nx, ny, i + 1, j, k);

  // Top
  if (j != ny - 1)
    neighbors[3] = elem_id<Hex8>(nx, ny, i, j + 1, k);

  // Left
  if (i != 0)
    neighbors[4] = elem_id<Hex8>(nx, ny, i - 1, j, k);

  // Front
  if (k != nz - 1)
    neighbors[5] = elem_id<Hex8>(nx, ny, i, j, k + 1);
}

template <>
inline dof_id_type
DistributedRectilinearMeshGenerator::node_id<Hex8>(const ElemType /*type*/,
                                                   const dof_id_type nx,
                                                   const dof_id_type ny,
                                                   const dof_id_type i,
                                                   const dof_id_type j,
                                                   const dof_id_type k)
{
  return i + (nx + 1) * (j + k * (ny + 1));
}

template <>
Node *
DistributedRectilinearMeshGenerator::add_point<Hex8>(const dof_id_type nx,
                                                     const dof_id_type ny,
                                                     const dof_id_type nz,
                                                     const dof_id_type i,
                                                     const dof_id_type j,
                                                     const dof_id_type k,
                                                     const ElemType type,
                                                     MeshBase & mesh)
{
  auto id = node_id<Hex8>(type, nx, ny, i, j, k);
  auto node_ptr = mesh.add_point(
      Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, static_cast<Real>(k) / nz), id);
  node_ptr->set_unique_id() = id;

  return node_ptr;
}

template <>
void
DistributedRectilinearMeshGenerator::add_element<Hex8>(const dof_id_type nx,
                                                       const dof_id_type ny,
                                                       const dof_id_type nz,
                                                       const dof_id_type i,
                                                       const dof_id_type j,
                                                       const dof_id_type k,
                                                       const dof_id_type elem_id,
                                                       const processor_id_type pid,
                                                       const ElemType type,
                                                       MeshBase & mesh,
                                                       bool verbose)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (verbose)
  {
    Moose::out << "Adding elem: " << elem_id << " pid: " << pid << std::endl;
    Moose::out << "Type: " << type << " " << HEX8 << std::endl;
  }

  // This ordering was picked to match the ordering in mesh_generation.C
  auto node0_ptr = add_point<Hex8>(nx, ny, nz, i, j, k, type, mesh);
  node0_ptr->processor_id() = pid;
  auto node1_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j, k, type, mesh);
  node1_ptr->processor_id() = pid;
  auto node2_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j + 1, k, type, mesh);
  node2_ptr->processor_id() = pid;
  auto node3_ptr = add_point<Hex8>(nx, ny, nz, i, j + 1, k, type, mesh);
  node3_ptr->processor_id() = pid;
  auto node4_ptr = add_point<Hex8>(nx, ny, nz, i, j, k + 1, type, mesh);
  node4_ptr->processor_id() = pid;
  auto node5_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j, k + 1, type, mesh);
  node5_ptr->processor_id() = pid;
  auto node6_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j + 1, k + 1, type, mesh);
  node6_ptr->processor_id() = pid;
  auto node7_ptr = add_point<Hex8>(nx, ny, nz, i, j + 1, k + 1, type, mesh);
  node7_ptr->processor_id() = pid;

  Elem * elem = new Hex8;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  elem->set_unique_id() = elem_id;
  elem = mesh.add_elem(elem);
  elem->set_node(0) = node0_ptr;
  elem->set_node(1) = node1_ptr;
  elem->set_node(2) = node2_ptr;
  elem->set_node(3) = node3_ptr;
  elem->set_node(4) = node4_ptr;
  elem->set_node(5) = node5_ptr;
  elem->set_node(6) = node6_ptr;
  elem->set_node(7) = node7_ptr;

  if (k == 0)
    boundary_info.add_side(elem, 0, 0);

  if (k == (nz - 1))
    boundary_info.add_side(elem, 5, 5);

  if (j == 0)
    boundary_info.add_side(elem, 1, 1);

  if (j == (ny - 1))
    boundary_info.add_side(elem, 3, 3);

  if (i == 0)
    boundary_info.add_side(elem, 4, 4);

  if (i == (nx - 1))
    boundary_info.add_side(elem, 2, 2);
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_indices<Hex8>(const dof_id_type nx,
                                                       const dof_id_type ny,
                                                       const dof_id_type elem_id,
                                                       dof_id_type & i,
                                                       dof_id_type & j,
                                                       dof_id_type & k)
{
  i = elem_id % nx;
  j = (((elem_id - i) / nx) % ny);
  k = ((elem_id - i) - (j * nx)) / (nx * ny);
}

template <>
inline void
DistributedRectilinearMeshGenerator::get_ghost_neighbors<Hex8>(const dof_id_type nx,
                                                               const dof_id_type ny,
                                                               const dof_id_type nz,
                                                               const MeshBase & mesh,
                                                               std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.get_boundary_info();

  dof_id_type i, j, k;

  std::vector<dof_id_type> neighbors(27);

  for (auto elem_ptr : mesh.element_ptr_range())
  {
    auto elem_id = elem_ptr->id();

    get_indices<Hex8>(nx, ny, elem_id, i, j, k);

    get_neighbors<Hex8>(nx, ny, nz, i, j, k, neighbors, true);

    for (auto neighbor : neighbors)
      if (neighbor != Elem::invalid_id && !mesh.query_elem_ptr(neighbor))
        ghost_elems.insert(neighbor);
  }
}

template <>
void
DistributedRectilinearMeshGenerator::set_boundary_names<Hex8>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "back";
  boundary_info.sideset_name(1) = "bottom";
  boundary_info.sideset_name(2) = "right";
  boundary_info.sideset_name(3) = "top";
  boundary_info.sideset_name(4) = "left";
  boundary_info.sideset_name(5) = "front";
}

template <>
void
DistributedRectilinearMeshGenerator::scale_nodal_positions<Hex8>(dof_id_type /*nx*/,
                                                                 dof_id_type /*ny*/,
                                                                 dof_id_type /*nz*/,
                                                                 Real xmin,
                                                                 Real xmax,
                                                                 Real ymin,
                                                                 Real ymax,
                                                                 Real zmin,
                                                                 Real zmax,
                                                                 MeshBase & mesh)
{
  for (auto & node_ptr : mesh.node_ptr_range())
  {
    (*node_ptr)(0) = (*node_ptr)(0) * (xmax - xmin) + xmin;
    (*node_ptr)(1) = (*node_ptr)(1) * (ymax - ymin) + ymin;
    (*node_ptr)(2) = (*node_ptr)(2) * (zmax - zmin) + zmin;
  }
}

template <typename T>
void
DistributedRectilinearMeshGenerator::build_cube(UnstructuredMesh & mesh,
                                                const unsigned int nx,
                                                unsigned int ny,
                                                unsigned int nz,
                                                const Real xmin,
                                                const Real xmax,
                                                const Real ymin,
                                                const Real ymax,
                                                const Real zmin,
                                                const Real zmax,
                                                const ElemType type,
                                                bool verbose)
{
  if (verbose)
    Moose::out << "nx: " << nx << "\n ny: " << ny << "\n nz: " << nz << std::endl;

  /// Here's the plan:
  /// 1. "Partition" the element linearly (i.e. break them up into n_procs contiguous chunks
  /// 2. Create a (dual) graph of the local elements
  /// 3. Partition the graph using Parmetis
  /// 4. Communicate the element IDs to the correct processors
  /// 5. Each processor creates only the elements it needs to
  /// 6. Figure out the ghosts we need
  /// 7. Request the PID of the ghosts and respond to requests for the same
  /// 8. Add ghosts to the mesh
  /// 9. ???
  /// 10. Profit!!!

  auto & comm = mesh.comm();

  dof_id_type num_elems = nx * ny * nz;

  if (verbose)
    Moose::out << "num elems: " << num_elems << std::endl;

  const auto num_procs = comm.size();
  // Current processor ID
  const auto pid = comm.rank();

  auto & boundary_info = mesh.get_boundary_info();

  std::unique_ptr<Elem> canonical_elem = libmesh_make_unique<T>();

  // Will get used to find the neighbors of an element
  std::vector<dof_id_type> neighbors(canonical_elem->n_neighbors());
  // Number of neighbors
  dof_id_type n_neighbors = canonical_elem->n_neighbors();

  if (verbose)
    Moose::out << "num neighbors: " << n_neighbors << std::endl;

  // "Partition" the elements linearly across the processors
  dof_id_type num_local_elems;
  dof_id_type local_elems_begin;
  dof_id_type local_elems_end;
  MooseUtils::linearPartitionItems(
      num_elems, num_procs, pid, num_local_elems, local_elems_begin, local_elems_end);

  std::vector<std::vector<dof_id_type>> graph;

  if (verbose)
    Moose::out << "num local elements: " << num_local_elems << std::endl;

  // Fill in xadj and adjncy
  // xadj is the offset into adjncy
  // adjncy are the face neighbors of each element on this processor
  graph.resize(num_local_elems);
  // Build a distributed graph
  num_local_elems = 0;
  for (dof_id_type e_id = local_elems_begin; e_id < local_elems_end; e_id++)
  {
    dof_id_type i, j, k;

    get_indices<T>(nx, ny, e_id, i, j, k);

    get_neighbors<T>(nx, ny, nz, i, j, k, neighbors);

    if (verbose)
      Moose::out << "e_id: " << e_id << std::endl;

    std::vector<dof_id_type> & row = graph[num_local_elems++];
    row.reserve(n_neighbors);

    for (auto neighbor : neighbors)
    {
      if (verbose)
        Moose::out << " neighbor: " << neighbor << std::endl;

      if (neighbor != Elem::invalid_id)
        row.push_back(neighbor);
    }
  }

  // Partition the distributed graph
  std::vector<dof_id_type> partition_vec;
  PetscExternalPartitioner::partitionGraph(
      comm, graph, {}, {}, num_procs, _num_parts_per_compute_node, _part_package, partition_vec);

  mooseAssert(partition_vec.size() == num_local_elems, " Invalid partition was generateed ");

  // Use current elements to remote processors according to partition
  std::map<processor_id_type, std::vector<dof_id_type>> pushed_elements_vecs;

  for (dof_id_type e_id = local_elems_begin; e_id < local_elems_end; e_id++)
  {
    pushed_elements_vecs[partition_vec[e_id - local_elems_begin]].push_back(e_id);
    if (verbose)
      Moose::out << " partition: (" << e_id << ", " << partition_vec[e_id - local_elems_begin]
                 << " )" << std::endl;
  }

  // Collect new elements I should own
  std::vector<dof_id_type> my_new_elems;

  auto elements_action_functor = [&my_new_elems](processor_id_type /*pid*/,
                                                 const std::vector<dof_id_type> & data) {
    std::copy(data.begin(), data.end(), std::back_inserter(my_new_elems));
  };

  Parallel::push_parallel_vector_data(comm, pushed_elements_vecs, elements_action_functor);

  // Add the elements this processor owns
  for (auto e_id : my_new_elems)
  {
    dof_id_type i, j, k;

    get_indices<T>(nx, ny, e_id, i, j, k);

    add_element<T>(nx, ny, nz, i, j, k, e_id, pid, type, mesh, verbose);

    if (verbose)
      Moose::out << " new element: " << e_id << std::endl;
  }

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        Moose::out << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                   << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Need to link up the local elements before we can know what's missing
  mesh.find_neighbors();

  if (verbose)
    Moose::out << "After first find_neighbors" << std::endl;

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        Moose::out << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                   << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Get the ghosts (missing face neighbors)
  std::set<dof_id_type> ghost_elems;
  get_ghost_neighbors<T>(nx, ny, nz, mesh, ghost_elems);

  // Elements we're going to request from others
  std::map<processor_id_type, std::vector<dof_id_type>> ghost_elems_to_request;

  for (auto & ghost_id : ghost_elems)
  {
    // This is the processor ID the ghost_elem was originally assigned to
    auto proc_id = MooseUtils::linearPartitionChunk(num_elems, num_procs, ghost_id);

    // Using side-effect insertion on purpose
    ghost_elems_to_request[proc_id].push_back(ghost_id);

    if (verbose)
      Moose::out << "ghost element " << ghost_id << " proc_id " << proc_id << std::endl;
  }

  // Next set ghost object ids from other processors
  auto gather_functor = [local_elems_begin, verbose, partition_vec](
                            processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & coming_ghost_elems,
                            std::vector<dof_id_type> & pid_for_ghost_elems) {
    auto num_ghost_elems = coming_ghost_elems.size();
    pid_for_ghost_elems.resize(num_ghost_elems);

    dof_id_type num_local_elems = 0;

    for (auto elem : coming_ghost_elems)
    {
      if (verbose)
        Moose::out << "coming ghost element " << elem << " proc_id "
                   << partition_vec[elem - local_elems_begin] << std::endl;

      pid_for_ghost_elems[num_local_elems++] = partition_vec[elem - local_elems_begin];
    }
  };

  std::unordered_map<dof_id_type, processor_id_type> ghost_elem_to_pid;

  auto action_functor =
      [verbose, &ghost_elem_to_pid](processor_id_type /*pid*/,
                                    const std::vector<dof_id_type> & my_ghost_elems,
                                    const std::vector<dof_id_type> & pid_for_my_ghost_elems) {
        dof_id_type num_local_elems = 0;

        for (auto elem : my_ghost_elems)
        {
          if (verbose)
            Moose::out << "my ghost element " << elem << " proc_id "
                       << pid_for_my_ghost_elems[num_local_elems] << std::endl;

          ghost_elem_to_pid[elem] = pid_for_my_ghost_elems[num_local_elems++];
        }
      };

  const dof_id_type * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm, ghost_elems_to_request, gather_functor, action_functor, ex);

  // Add the ghost elements to the mesh
  for (auto gtop : ghost_elem_to_pid)
  {
    auto ghost_id = gtop.first;
    auto proc_id = gtop.second;

    dof_id_type i, j, k;

    get_indices<T>(nx, ny, ghost_id, i, j, k);

    add_element<T>(nx, ny, nz, i, j, k, ghost_id, proc_id, type, mesh, verbose);
  }

  if (verbose)
    Moose::out << "After adding ghosts" << std::endl;

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        Moose::out << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                   << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  mesh.find_neighbors(true);

  if (verbose)
    Moose::out << "After second find neighbors " << std::endl;

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        Moose::out << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                   << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Set RemoteElem neighbors
  for (auto & elem_ptr : mesh.element_ptr_range())
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
      if (!elem_ptr->neighbor_ptr(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
        elem_ptr->set_neighbor(s, const_cast<RemoteElem *>(remote_elem));

  if (verbose)
    Moose::out << "After adding remote elements" << std::endl;

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        Moose::out << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                   << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  set_boundary_names<T>(boundary_info);

  Partitioner::set_node_processor_ids(mesh);

  if (verbose)
    Moose::out << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  if (verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      Moose::out << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  // Already partitioned!
  mesh.skip_partitioning(true);

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      Moose::out << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id()
                 << " uid: " << elem_ptr->unique_id() << std::endl;

  if (verbose)
    Moose::out << "Getting ready to prepare for use" << std::endl;

  mesh.prepare_for_use(true, true); // No need to renumber or find neighbors - done did it.

  if (verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      Moose::out << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id() << std::endl;

  if (verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      Moose::out << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  if (verbose)
    Moose::out << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  // Scale the nodal positions
  scale_nodal_positions<T>(nx, ny, nz, xmin, xmax, ymin, ymax, zmin, zmax, mesh);

  if (verbose)
    mesh.print_info();
}

std::unique_ptr<MeshBase>
DistributedRectilinearMeshGenerator::generate()
{
  // DistributedRectilinearMeshGenerator always generates a distributed mesh
  _mesh->setParallelType(MooseMesh::ParallelType::DISTRIBUTED);
  auto mesh = _mesh->buildMeshBaseObject(MooseMesh::ParallelType::DISTRIBUTED);

  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
      case 1:
        elem_type_enum = "EDGE2";
        break;
      case 2:
        elem_type_enum = "QUAD4";
        break;
      case 3:
        elem_type_enum = "HEX8";
        break;
    }
  }

  _elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  mesh->set_mesh_dimension(_dim);
  mesh->set_spatial_dimension(_dim);

  // Switching on MooseEnum
  switch (_dim)
  {
    // The build_XYZ mesh generation functions take an
    // UnstructuredMesh& as the first argument, hence the dynamic_cast.
    case 1:
      build_cube<Edge2>(dynamic_cast<UnstructuredMesh &>(*mesh),
                        _nx,
                        1,
                        1,
                        _xmin,
                        _xmax,
                        0,
                        0,
                        0,
                        0,
                        _elem_type,
                        _verbose);
      break;
    case 2:
      build_cube<Quad4>(dynamic_cast<UnstructuredMesh &>(*mesh),
                        _nx,
                        _ny,
                        1,
                        _xmin,
                        _xmax,
                        _ymin,
                        _ymax,
                        0,
                        0,
                        _elem_type,
                        _verbose);
      break;
    case 3:
      build_cube<Hex8>(dynamic_cast<UnstructuredMesh &>(*mesh),
                       _nx,
                       _ny,
                       _nz,
                       _xmin,
                       _xmax,
                       _ymin,
                       _ymax,
                       _zmin,
                       _zmax,
                       _elem_type,
                       _verbose);
      break;
    default:
      mooseError(getParam<MooseEnum>("elem_type"),
                 " is not a currently supported element type for DistributedGeneratedMesh");
  }

  // Apply the bias if any exists
  if (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0)
  {
    // Biases
    Real bias[3] = {_bias_x, _bias_y, _bias_z};

    // "width" of the mesh in each direction
    Real width[3] = {_xmax - _xmin, _ymax - _ymin, _zmax - _zmin};

    // Min mesh extent in each direction.
    Real mins[3] = {_xmin, _ymin, _zmin};

    // Number of elements in each direction.
    dof_id_type nelem[3] = {_nx, _ny, _nz};

    // We will need the biases raised to integer powers in each
    // direction, so let's pre-compute those...
    std::vector<std::vector<Real>> pows(LIBMESH_DIM);
    for (dof_id_type dir = 0; dir < LIBMESH_DIM; ++dir)
    {
      pows[dir].resize(nelem[dir] + 1);
      for (dof_id_type i = 0; i < pows[dir].size(); ++i)
        pows[dir][i] = std::pow(bias[dir], static_cast<int>(i));
    }

    // Loop over the nodes and move them to the desired location
    for (auto & node_ptr : mesh->node_ptr_range())
    {
      Node & node = *node_ptr;

      for (dof_id_type dir = 0; dir < LIBMESH_DIM; ++dir)
      {
        if (width[dir] != 0. && bias[dir] != 1.)
        {
          // Compute the scaled "index" of the current point.  This
          // will either be close to a whole integer or a whole
          // integer+0.5 for quadratic nodes.
          Real float_index = (node(dir) - mins[dir]) * nelem[dir] / width[dir];

          Real integer_part = 0;
          Real fractional_part = std::modf(float_index, &integer_part);

          // Figure out where to move the node...
          if (std::abs(fractional_part) < TOLERANCE || std::abs(fractional_part - 1.0) < TOLERANCE)
          {
            // If the fractional_part ~ 0.0 or 1.0, this is a vertex node, so
            // we don't need an average.
            //
            // Compute the "index" we are at in the current direction.  We
            // round to the nearest integral value to get this instead
            // of using "integer_part", since that could be off by a
            // lot (e.g. we want 3.9999 to map to 4.0 instead of 3.0).
            int index = round(float_index);

            // Move node to biased location.
            node(dir) =
                mins[dir] + width[dir] * (1. - pows[dir][index]) / (1. - pows[dir][nelem[dir]]);
          }
          else if (std::abs(fractional_part - 0.5) < TOLERANCE)
          {
            // If the fractional_part ~ 0.5, this is a midedge/face
            // (i.e. quadratic) node.  We don't move those with the same
            // bias as the vertices, instead we put them midway between
            // their respective vertices.
            //
            // Also, since the fractional part is nearly 0.5, we know that
            // the integer_part will be the index of the vertex to the
            // left, and integer_part+1 will be the index of the
            // vertex to the right.
            node(dir) = mins[dir] +
                        width[dir] *
                            (1. - 0.5 * (pows[dir][integer_part] + pows[dir][integer_part + 1])) /
                            (1. - pows[dir][nelem[dir]]);
          }
          else
          {
            // We don't yet handle anything higher order than quadratic...
            mooseError("Unable to bias node at node(", dir, ")=", node(dir));
          }
        }
      }
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
