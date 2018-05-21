//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedGeneratedMesh.h"

#include "SerializerGuard.h"

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
#include "libmesh/parmetis_helper.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

#ifdef LIBMESH_HAVE_METIS
// MIPSPro 7.4.2 gets confused about these nested namespaces
#ifdef __sgi
#include <cstdarg>
#endif
namespace Metis
{
extern "C" {
#include "libmesh/ignore_warnings.h"
#include "metis.h"
#include "libmesh/restore_warnings.h"
}
}
#else
#include "libmesh/sfc_partitioner.h"
#endif

registerMooseObject("MooseApp", DistributedGeneratedMesh);

template <>
InputParameters
validParams<DistributedGeneratedMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addParam<bool>("verbose", false, "Turn on verbose printing for the mesh generation");

  MooseEnum elem_types(
      "EDGE EDGE2 EDGE3 EDGE4 QUAD QUAD4 QUAD8 QUAD9 TRI3 TRI6 HEX HEX8 HEX20 HEX27 TET4 TET10 "
      "PRISM6 PRISM15 PRISM18 PYRAMID5 PYRAMID13 PYRAMID14"); // no default

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

  // This mesh is always distributed
  params.set<MooseEnum>("parallel_type") = "DISTRIBUTED";

  return params;
}

DistributedGeneratedMesh::DistributedGeneratedMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _verbose(getParam<bool>("verbose")),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<dof_id_type>("nx")),
    _ny(getParam<dof_id_type>("ny")),
    _nz(getParam<dof_id_type>("nz")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z"))
{
  // All generated meshes are regular orthogonal meshes
  _regular_orthogonal_mesh = true;
}

Real
DistributedGeneratedMesh::getMinInDimension(dof_id_type component) const
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
DistributedGeneratedMesh::getMaxInDimension(dof_id_type component) const
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

MooseMesh &
DistributedGeneratedMesh::clone() const
{
  return *(new DistributedGeneratedMesh(*this));
}

namespace
{

/**
 * Get the element ID for a given hex
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @param k The z index of this element
 * @return The ID of the i,j element
 */
template <typename T>
inline dof_id_type
elem_id(const dof_id_type /*nx*/,
        const dof_id_type /*ny*/,
        const dof_id_type /*i*/,
        const dof_id_type /*j*/,
        const dof_id_type /*k*/)
{
  mooseError("elem_id not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Get the number of neighbors this element will have
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param nz The number of elements in the z direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @param k The z index of this element
 * @return The number of neighboring elements
 */
template <typename T>
inline dof_id_type
num_neighbors(const dof_id_type /*nx*/,
              const dof_id_type /*ny*/,
              const dof_id_type /*nz*/,
              const dof_id_type /*i*/,
              const dof_id_type /*j*/,
              const dof_id_type /*k*/)
{
  mooseError("num_neighbors not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Get the IDs of the neighbors of a given element
 *
 * @param nx The number of elements in the x direction
 * @param nx The number of elements in the y direction
 * @param nz The number of elements in the z direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @param k The z index of this element
 * @param neighbors This will be filled with the IDs of the two neighbors or invalid_dof_id if there
 * is no neighbor.  THIS MUST be of size 6 BEFORE calling this function
 */
template <typename T>
inline void
get_neighbors(const dof_id_type /*nx*/,
              const dof_id_type /*ny*/,
              const dof_id_type /*nz*/,
              const dof_id_type /*i*/,
              const dof_id_type /*j*/,
              const dof_id_type /*k*/,
              std::vector<dof_id_type> & /*neighbors*/)
{
  mooseError("get_neighbors not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * The ID of the i,j,k node
 *
 * @param type The element type
 * @param nx The number of elements in the x direction
 * @param nx The number of elements in the y direction
 * @param nz The number of elements in the z direction
 * @param i The x index of this node
 * @param j The y index of this node
 * @param k The z index of this node
 */
template <typename T>
inline dof_id_type
node_id(const ElemType /*type*/,
        const dof_id_type /*nx*/,
        const dof_id_type /*ny*/,
        const dof_id_type /*i*/,
        const dof_id_type /*j*/,
        const dof_id_type /*k*/)
{
  mooseError("node_id not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Add a node to the mesh
 *
 * @param nx The number of elements in the x direction
 * @param nx The number of elements in the y direction
 * @param nz The number of elements in the z direction
 * @param i The x index of this node
 * @param j The y index of this node
 * @param k The z index of this node
 * @param type The element type
 * @param mesh The mesh to add it to
 */
template <typename T>
Node *
add_point(const dof_id_type /*nx*/,
          const dof_id_type /*ny*/,
          const dof_id_type /*nz*/,
          const dof_id_type /*i*/,
          const dof_id_type /*j*/,
          const dof_id_type /*k*/,
          const ElemType /*type*/,
          MeshBase & /*mesh*/)
{
  mooseError("add_point not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Adds an element to the mesh
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param nz The number of elements in the z direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @param k The z index of this element
 * @param elem_id The element ID of the element to add
 * @param pid The processor ID to assign it to
 * @param type The type of element to add
 * @param mesh The mesh to add it to
 * @param verbose Whether or not to print out verbose statements
 */
template <typename T>
void
add_element(const dof_id_type /*nx*/,
            const dof_id_type /*ny*/,
            const dof_id_type /*nz*/,
            const dof_id_type /*i*/,
            const dof_id_type /*j*/,
            const dof_id_type /*k*/,
            const dof_id_type /*elem_id*/,
            const processor_id_type /*pid*/,
            const ElemType /*type*/,
            MeshBase & /*mesh*/,
            bool /*verbose*/)
{
  mooseError("add_element not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Compute the i,j,k indices of a given element ID
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param elem_id The ID of the element
 * @param i Output: The index in the x direction
 * @param j Output: The index in the y direction
 * @param k Output: The index in the z direction
 */
template <typename T>
inline void
get_indices(const dof_id_type /*nx*/,
            const dof_id_type /*ny*/,
            const dof_id_type /*elem_id*/,
            dof_id_type & /*i*/,
            dof_id_type & /*j*/,
            dof_id_type & /*k*/)
{
  mooseError("get_indices not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Find the elements and sides that need ghost elements
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param mesh The mesh - without any ghost elements
 * @param ghost_elems The ghost elems that need to be added
 */
template <typename T>
inline void
get_ghost_neighbors(const dof_id_type /*nx*/,
                    const dof_id_type /*ny*/,
                    const dof_id_type /*nz*/,
                    const MeshBase & /*mesh*/,
                    std::set<dof_id_type> & /*ghost_elems*/)
{
  mooseError(
      "get_ghost_neighbors not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * Set the boundary names for any added boundary ideas
 *
 * @boundary_info The BoundaryInfo object to set the boundary names on
 */
template <typename T>
void
set_boundary_names(BoundaryInfo & /*boundary_info*/)
{
  mooseError(
      "set_boundary_names not implemented for this element type in DistributedGeneratedMesh");
}

/**
 * All meshes are generated on the unit square.  This function stretches the mesh
 * out to fill the correct area.
 */
template <typename T>
void
scale_nodal_positions(dof_id_type /*nx*/,
                      dof_id_type /*ny*/,
                      dof_id_type /*nz*/,
                      Real /*xmin*/,
                      Real /*xmax*/,
                      Real /*ymin*/,
                      Real /*ymax*/,
                      Real /*zmin*/,
                      Real /*zmax*/,
                      MeshBase & /*mesh*/)
{
  mooseError(
      "scale_nodal_positions not implemented for this element type in DistributedGeneratedMesh");
}

template <>
inline dof_id_type
num_neighbors<Edge2>(const dof_id_type nx,
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

template <>
inline void
get_neighbors<Edge2>(const dof_id_type nx,
                     const dof_id_type /*ny*/,
                     const dof_id_type /*nz*/,
                     const dof_id_type i,
                     const dof_id_type /*j*/,
                     const dof_id_type /*k*/,
                     std::vector<dof_id_type> & neighbors)

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
get_indices<Edge2>(const dof_id_type /*nx*/,
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
get_ghost_neighbors<Edge2>(const dof_id_type nx,
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
elem_id<Edge2>(const dof_id_type /*nx*/,
               const dof_id_type /*ny*/,
               const dof_id_type i,
               const dof_id_type /*j*/,
               const dof_id_type /*k*/)
{
  return i;
}

template <>
void
add_element<Edge2>(const dof_id_type nx,
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

  auto node1_ptr =
      mesh.add_point(Point(static_cast<Real>(node_offset + 1) / nx, 0, 0), node_offset + 1);
  node1_ptr->set_unique_id() = node_offset + 1;

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
set_boundary_names<Edge2>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "left";
  boundary_info.sideset_name(1) = "right";
}

template <>
void
scale_nodal_positions<Edge2>(dof_id_type /*nx*/,
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
num_neighbors<Quad4>(const dof_id_type nx,
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
elem_id<Quad4>(const dof_id_type nx,
               const dof_id_type /*nx*/,
               const dof_id_type i,
               const dof_id_type j,
               const dof_id_type /*k*/)
{
  return (j * nx) + i;
}

template <>
inline void
get_neighbors<Quad4>(const dof_id_type nx,
                     const dof_id_type ny,
                     const dof_id_type /*nz*/,
                     const dof_id_type i,
                     const dof_id_type j,
                     const dof_id_type /*k*/,
                     std::vector<dof_id_type> & neighbors)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

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
get_indices<Quad4>(const dof_id_type nx,
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
get_ghost_neighbors<Quad4>(const dof_id_type nx,
                           const dof_id_type ny,
                           const dof_id_type /*nz*/,
                           const MeshBase & mesh,
                           std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.get_boundary_info();

  dof_id_type i, j, k;

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

          get_indices<Quad4>(nx, 0, elem_id, i, j, k);

          get_neighbors<Quad4>(nx, ny, 0, i, j, 0, neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}

template <>
inline dof_id_type
node_id<Quad4>(const ElemType /*type*/,
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
add_element<Quad4>(const dof_id_type nx,
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

  // Bottom Right
  auto node1_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j) / ny, 0),
                     node_id<Quad4>(type, nx, 0, i + 1, j, 0));
  node1_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i + 1, j, 0);

  // Top Right
  auto node2_ptr =
      mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id<Quad4>(type, nx, 0, i + 1, j + 1, 0));
  node2_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i + 1, j + 1, 0);

  // Top Left
  auto node3_ptr =
      mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j + 1) / ny, 0),
                     node_id<Quad4>(type, nx, 0, i, j + 1, 0));
  node3_ptr->set_unique_id() = node_id<Quad4>(type, nx, 0, i, j + 1, 0);

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
set_boundary_names<Quad4>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "right";
  boundary_info.sideset_name(2) = "top";
  boundary_info.sideset_name(3) = "left";
}

template <>
void
scale_nodal_positions<Quad4>(dof_id_type /*nx*/,
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
elem_id<Hex8>(const dof_id_type nx,
              const dof_id_type ny,
              const dof_id_type i,
              const dof_id_type j,
              const dof_id_type k)
{
  return i + (j * nx) + (k * nx * ny);
}

template <>
inline dof_id_type
num_neighbors<Hex8>(const dof_id_type nx,
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
get_neighbors<Hex8>(const dof_id_type nx,
                    const dof_id_type ny,
                    const dof_id_type nz,
                    const dof_id_type i,
                    const dof_id_type j,
                    const dof_id_type k,
                    std::vector<dof_id_type> & neighbors)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

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
node_id<Hex8>(const ElemType /*type*/,
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
add_point<Hex8>(const dof_id_type nx,
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
add_element<Hex8>(const dof_id_type nx,
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
  auto node1_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j, k, type, mesh);
  auto node2_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j + 1, k, type, mesh);
  auto node3_ptr = add_point<Hex8>(nx, ny, nz, i, j + 1, k, type, mesh);
  auto node4_ptr = add_point<Hex8>(nx, ny, nz, i, j, k + 1, type, mesh);
  auto node5_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j, k + 1, type, mesh);
  auto node6_ptr = add_point<Hex8>(nx, ny, nz, i + 1, j + 1, k + 1, type, mesh);
  auto node7_ptr = add_point<Hex8>(nx, ny, nz, i, j + 1, k + 1, type, mesh);

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
get_indices<Hex8>(const dof_id_type nx,
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
get_ghost_neighbors<Hex8>(const dof_id_type nx,
                          const dof_id_type ny,
                          const dof_id_type nz,
                          const MeshBase & mesh,
                          std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.get_boundary_info();

  dof_id_type i, j, k;

  std::vector<dof_id_type> neighbors(6);

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

          get_indices<Hex8>(nx, ny, elem_id, i, j, k);

          get_neighbors<Hex8>(nx, ny, nz, i, j, k, neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}

template <>
void
set_boundary_names<Hex8>(BoundaryInfo & boundary_info)
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
scale_nodal_positions<Hex8>(dof_id_type /*nx*/,
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
}

template <typename T>
void
build_cube(UnstructuredMesh & mesh,
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
  const auto num_procs = comm.size();
  const auto pid = comm.rank();

  auto & boundary_info = mesh.get_boundary_info();

  auto elem_tag = comm.get_unique_tag(934);
  auto ghost_request_tag = comm.get_unique_tag(832);
  auto ghost_response_tag = comm.get_unique_tag(821);

  std::unique_ptr<Elem> canonical_elem = libmesh_make_unique<T>();

  // Will get used to find the neighbors of an element
  std::vector<dof_id_type> neighbors(canonical_elem->n_neighbors());

  // "Partition" the elements linearly across the processors
  dof_id_type num_local_elems;
  dof_id_type local_elems_begin;
  dof_id_type local_elems_end;
  MooseUtils::linearPartitionItems(
      num_elems, num_procs, pid, num_local_elems, local_elems_begin, local_elems_end);

  std::unique_ptr<ParmetisHelper> pmetis = libmesh_make_unique<ParmetisHelper>();

  // Set parameters.
  pmetis->wgtflag = 2;                                      // weights on vertices only
  pmetis->ncon = 1;                                         // one weight per vertex
  pmetis->numflag = 0;                                      // C-style 0-based numbering
  pmetis->nparts = static_cast<Parmetis::idx_t>(num_procs); // number of subdomains to create
  pmetis->edgecut = 0;                                      // the numbers of edges cut by the
                                                            // partition
  // Initialize data structures for ParMETIS
  pmetis->xadj.assign(num_local_elems + 1, 0);
  // Size this to the max it can be
  pmetis->adjncy.assign((num_local_elems * canonical_elem->n_neighbors()), 0);
  pmetis->vtxdist.assign(num_procs + 1, 0);
  pmetis->tpwgts.assign(pmetis->nparts, 1. / pmetis->nparts);
  pmetis->ubvec.assign(pmetis->ncon, 1.05);
  pmetis->part.assign(num_local_elems, 0);
  pmetis->options.resize(5);
  pmetis->vwgt.assign(num_local_elems, canonical_elem->n_nodes());

  // Set the options
  pmetis->options[0] = 1;  // don't use default options
  pmetis->options[1] = 0;  // default (level of timing)
  pmetis->options[2] = 15; // random seed (defaul)t
  pmetis->options[3] = 2;  // processor distribution and subdomain distribution are decoupled

  // Fill vtxdist.  These are the bin edges of the ranges current on each proc
  // Note that this was resized to "num_procs + 1" up above
  auto & vtxdist = pmetis->vtxdist;
  for (processor_id_type p = 0; p < num_procs; p++)
  {
    dof_id_type t_num_local_elems;
    dof_id_type t_local_elems_begin;
    dof_id_type t_local_elems_end;
    MooseUtils::linearPartitionItems(
        num_elems, num_procs, p, t_num_local_elems, t_local_elems_begin, t_local_elems_end);

    vtxdist[p] = t_local_elems_begin;

    if (verbose)
      Moose::out << "t_local_elems_begin: " << t_local_elems_begin << std::endl;

    // The last one needs to fill in the final entry too
    if (p == num_procs - 1)
      vtxdist[p + 1] = t_local_elems_end;
  }

  // Fill in xadj and adjncy
  // xadj is the offset into adjncy
  // adjncy are the face neighbors of each element on this processor
  auto & xadj = pmetis->xadj;
  auto & adjncy = pmetis->adjncy;

  dof_id_type local_elem = 0;
  dof_id_type offset = 0;

  for (dof_id_type e_id = local_elems_begin; e_id < local_elems_end; e_id++)
  {
    xadj[local_elem] = offset;

    dof_id_type i, j, k;

    get_indices<T>(nx, ny, e_id, i, j, k);

    get_neighbors<T>(nx, ny, nz, i, j, k, neighbors);

    if (verbose)
      Moose::out << "e_id: " << e_id << std::endl;

    for (auto neighbor : neighbors)
    {
      if (verbose)
        Moose::out << " neighbor: " << neighbor << std::endl;

      if (neighbor != Elem::invalid_id)
        adjncy[offset++] = neighbor;
    }

    local_elem++;
  }

  // Fill in the last entry
  xadj[local_elem] = offset;

  if (verbose)
  {
    Moose::out << "xadj: ";
    for (auto val : xadj)
      Moose::out << val << " ";
    Moose::out << std::endl;

    Moose::out << "adjncy: ";
    for (auto val : adjncy)
      Moose::out << val << " ";
    Moose::out << std::endl;

    Moose::out << "vtxdist: ";
    for (auto val : vtxdist)
      Moose::out << val << " ";
    Moose::out << std::endl;
  }

  // Fill in the last entry
  xadj[num_local_elems + 1] = adjncy.size() + 1;

  if (num_procs == 1)
  {
    // Just assign them all to proc 0
    for (auto & elem_pid : pmetis->part)
      elem_pid = 0;
  }
  else
  {
    std::vector<Parmetis::idx_t> vsize(pmetis->vwgt.size(), 1);
    Parmetis::real_t itr = 1000000.0;
    MPI_Comm mpi_comm = comm.get();

    Parmetis::ParMETIS_V3_AdaptiveRepart(
        pmetis->vtxdist.empty() ? libmesh_nullptr : &pmetis->vtxdist[0],
        pmetis->xadj.empty() ? libmesh_nullptr : &pmetis->xadj[0],
        pmetis->adjncy.empty() ? libmesh_nullptr : &pmetis->adjncy[0],
        pmetis->vwgt.empty() ? libmesh_nullptr : &pmetis->vwgt[0],
        vsize.empty() ? libmesh_nullptr : &vsize[0],
        libmesh_nullptr,
        &pmetis->wgtflag,
        &pmetis->numflag,
        &pmetis->ncon,
        &pmetis->nparts,
        pmetis->tpwgts.empty() ? libmesh_nullptr : &pmetis->tpwgts[0],
        pmetis->ubvec.empty() ? libmesh_nullptr : &pmetis->ubvec[0],
        &itr,
        &pmetis->options[0],
        &pmetis->edgecut,
        pmetis->part.empty() ? libmesh_nullptr : &pmetis->part[0],
        &mpi_comm);
  }

  if (verbose)
  {
    Moose::out << "Part " << comm.rank() << ": ";
    for (auto apid : pmetis->part)
      Moose::out << apid << " ";
    Moose::out << std::endl;
  }

  // Partitioning is complete.
  // Now we need to tell each processor which elements
  // it will be handling.  It'll go like this:
  // 1.  AlltoAll to figure out who's sending/receiving and how many elements
  //     Note: This AlltoAll could be removed in the future using
  //     a super nonblocking scheme
  // 2.  Post non-blocking sends
  // 3.  Receive
  // 4.  Closeup all Sends

  // The complete list of elements assigned to this proc:
  std::vector<dof_id_type> my_elems;

  // Will hold the number of elements we're going to send to each processor
  std::vector<dof_id_type> will_send_to(num_procs, 0);

  // Keep track of the total number of local elements
  // This will be the number we know about in our local "part" vector
  // plus the number other people will send to us
  dof_id_type total_num_local_elems = 0;

  // The actual vectors to be sent
  std::map<processor_id_type, std::vector<dof_id_type>> elems_to_be_sent;

  // Total number of messages we'll send
  processor_id_type num_sends = 0;

  dof_id_type current_elem_id = local_elems_begin;

  for (auto proc_id : pmetis->part)
  {
    if (static_cast<processor_id_type>(proc_id) != pid)
    {
      // Keep track of the unique procs we're sending to
      if (!will_send_to[proc_id])
        num_sends++;

      will_send_to[proc_id]++;

      // Side-effect insertion used on purpose
      //
      // Note: I thought about the speed here, I could
      // find the number of elems going to each processor
      // first so I could reserve the correct space in the
      // vectors.  But when I went to implement that it
      // simply wasn't worth the increase in code complexity
      elems_to_be_sent[proc_id].push_back(current_elem_id);
    }
    else
    {
      total_num_local_elems++;
      my_elems.push_back(current_elem_id);
    }

    current_elem_id++;
  }

  // Trade data
  comm.alltoall(will_send_to);

  // will_send_to now represents who we'll receive from
  // give it a good name
  auto & will_receive_from = will_send_to;

  // Post the sends
  std::vector<Parallel::Request> elem_sends(num_sends);

  processor_id_type current_send = 0;
  for (auto & pid_elems_to_be_sent : elems_to_be_sent)
  {
    auto proc_id = pid_elems_to_be_sent.first;
    auto & elems_to_send = pid_elems_to_be_sent.second;

    comm.send(proc_id, elems_to_send, elem_sends[current_send], elem_tag);

    current_send++;
  }

  // Now start receiving

  // Count the number of receives and the total number of elements
  processor_id_type num_receives = 0;
  for (processor_id_type proc_id = 0; proc_id < num_procs; proc_id++)
  {
    if (will_receive_from[proc_id])
    {
      num_receives++;
      total_num_local_elems += will_receive_from[proc_id];
    }
  }

  // Note that my_elems might already have some stuff in it
  // But let's go ahead and make sure it has enough to hold
  // that stuff and everything that is received.
  my_elems.reserve(total_num_local_elems);

  // Will hold incoming elem_ids
  std::vector<dof_id_type> in_elems;

  // Post the receives
  for (processor_id_type current_receive = 0; current_receive < num_receives; current_receive++)
  {
    // It doesn't matter what order we process the receives in
    comm.receive(Parallel::any_source, in_elems, elem_tag);

    for (auto & e_id : in_elems)
      my_elems.push_back(e_id);
  }

  // Finish the sends
  Parallel::wait(elem_sends);

  // Add the elements this processor owns
  for (auto & e_id : my_elems)
  {
    dof_id_type i, j, k;

    get_indices<T>(nx, ny, e_id, i, j, k);

    add_element<T>(nx, ny, nz, i, j, k, e_id, pid, type, mesh, verbose);
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

  // Now at this point we know the ghosts we need to add but we don't know
  // the processor ID they ended up on.  Here's what we're going to
  // do about it:
  // 1. Figure out which processor originally had the ghost elements
  // 2. Send a request to each of those processors
  // 3. Receive back the place the ghosts ended up
  // 4. Add the ghosts with the correct PID

  // Who we're going to make requests of
  std::vector<char> request_ghosts_from(num_procs, 0);

  // Elements we're going to request from others
  std::map<processor_id_type, std::vector<dof_id_type>> ghost_elems_to_request;

  for (auto & ghost_id : ghost_elems)
  {
    // This is the processor ID the ghost_elem was originally assigned to
    auto proc_id = MooseUtils::linearPartitionChunk(num_elems, num_procs, ghost_id);

    request_ghosts_from[proc_id] = true;

    // Using side-effect insertion on purpose
    ghost_elems_to_request[proc_id].push_back(ghost_id);
  }

  // Tell everyone who we're going to request from
  comm.alltoall(request_ghosts_from);

  // Now request_ghosts_from tells us who is requesting ghosts from _us_
  auto & send_ghosts_to = request_ghosts_from;

  // Five Phases:
  // 1. Send off _my_ requests
  // 2. Receive requests from others
  // 3. Fill requests from others
  // 4. Send results back
  // 5. Receive and process _my_ requests

  // 1. Send off _my_ requests

  std::vector<Parallel::Request> ghost_request_sends(ghost_elems_to_request.size());

  processor_id_type current_ghost_request_send = 0;
  for (auto & proc_id_ghost_elem : ghost_elems_to_request)
  {
    auto proc_id = proc_id_ghost_elem.first;
    auto & ghosts_to_send = proc_id_ghost_elem.second;

    comm.send(proc_id,
              ghosts_to_send,
              ghost_request_sends[current_ghost_request_send],
              ghost_request_tag);

    current_ghost_request_send++;
  }

  // 2. Start receiving requests

  // 2a: Figure out how many requests we'll be receiving
  processor_id_type number_of_incoming_requests = 0;
  for (auto val : send_ghosts_to)
    if (val)
      number_of_incoming_requests++;

  // Will hold the PID for each requests ghost element so we can send them back
  std::map<processor_id_type, std::vector<dof_id_type>> filled_requests_to_send_back;

  // Will hold incoming ghost_ids
  std::vector<dof_id_type> in_ghosts;

  // 2b. and 3. Receive requests and process them
  for (processor_id_type current_incoming_request = 0;
       current_incoming_request < number_of_incoming_requests;
       current_incoming_request++)
  {
    // The stat will tell where this message came from - so we'll know where to send the response to
    auto stat = comm.receive(Parallel::any_source, in_ghosts, ghost_request_tag);

    auto proc_id = stat.source();

    // Side effect insertion on purpose
    auto & proc_ids_of_ghosts = filled_requests_to_send_back[proc_id];

    proc_ids_of_ghosts.reserve(in_ghosts.size());

    // Fill up the vector we will send back with the PID the ghost was assigned to
    for (auto & ghost_id : in_ghosts)
    {
      auto local_id = ghost_id - local_elems_begin;

      mooseAssert(local_id < pmetis->part.size(),
                  "Invalid request! Received a request for an element that "
                  "doesn't exist on this processor in "
                  "DistributedGeneratedMesh");

      proc_ids_of_ghosts.push_back(pmetis->part[local_id]);
    }
  }

  // 4. Send responses back
  std::vector<Parallel::Request> ghost_response_sends(filled_requests_to_send_back.size());

  processor_id_type current_response_send = 0;
  for (auto & proc_id_response : filled_requests_to_send_back)
  {
    auto proc_id = proc_id_response.first;
    auto & response_to_send = proc_id_response.second;

    comm.send(proc_id, response_to_send, ghost_response_sends[current_send], ghost_response_tag);

    current_response_send++;
  }

  // 5. Receive and process _my_ responses

  // This will hold the PID for each ghost element that was requested from a processor
  std::vector<dof_id_type> in_response;

  for (processor_id_type current_incoming_response = 0;
       current_incoming_response < ghost_elems_to_request.size();
       current_incoming_response++)
  {
    auto stat = comm.receive(Parallel::any_source, in_response, ghost_response_tag);

    auto proc_id = stat.source();

    // Grab the ghost_elems we requested from this processor
    auto & ghost_elems = ghost_elems_to_request[proc_id];

    // Add the ghost elements to the mesh
    for (dof_id_type current_ghost_elem = 0; current_ghost_elem < ghost_elems.size();
         current_ghost_elem++)
    {
      auto ghost_id = ghost_elems[current_ghost_elem];
      auto proc_id = in_response[current_ghost_elem];

      dof_id_type i, j, k;

      get_indices<T>(nx, ny, ghost_id, i, j, k);

      add_element<T>(nx, ny, nz, i, j, k, ghost_id, proc_id, type, mesh, verbose);
    }
  }

  // Tie up loose ends
  Parallel::wait(ghost_request_sends);
  Parallel::wait(ghost_response_sends);

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
      if (!elem_ptr->neighbor(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
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

void
DistributedGeneratedMesh::buildMesh()
{
  // Reference to the libmesh mesh
  MeshBase & mesh = getMesh();

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

  mesh.set_mesh_dimension(_dim);
  mesh.set_spatial_dimension(_dim);

  // Switching on MooseEnum
  switch (_dim)
  {
    // The build_XYZ mesh generation functions take an
    // UnstructuredMesh& as the first argument, hence the dynamic_cast.
    case 1:
      build_cube<Edge2>(dynamic_cast<UnstructuredMesh &>(getMesh()),
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
      build_cube<Quad4>(dynamic_cast<UnstructuredMesh &>(getMesh()),
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
      build_cube<Hex8>(dynamic_cast<UnstructuredMesh &>(getMesh()),
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
    for (auto & node_ptr : mesh.node_ptr_range())
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
}
