//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedGeneratedMesh.h"

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

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

#ifdef LIBMESH_HAVE_METIS
// MIPSPro 7.4.2 gets confused about these nested namespaces
#ifdef __sgi
#include <cstdarg>
#endif
namespace Metis
{
extern "C"
{
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
    _bias_z(getParam<Real>("bias_z")),
    _dims_may_have_changed(false)
{
  // All generated meshes are regular orthogonal meshes
  _regular_orthogonal_mesh = true;
}

void
DistributedGeneratedMesh::prepared(bool state)
{
  MooseMesh::prepared(state);

  // Fall back on scanning the mesh for coordinates instead of using input parameters for queries
  if (!state)
    _dims_may_have_changed = true;
}

Real
DistributedGeneratedMesh::getMinInDimension(unsigned int component) const
{
  if (_dims_may_have_changed)
    return MooseMesh::getMinInDimension(component);

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
DistributedGeneratedMesh::getMaxInDimension(unsigned int component) const
{
  if (_dims_may_have_changed)
    return MooseMesh::getMaxInDimension(component);

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
DistributedGeneratedMesh::safeClone() const
{
  return libmesh_make_unique<DistributedGeneratedMesh>(*this);
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
  /// 1. Create a (dual) graph of the elements
  /// 2. Partition the graph
  /// 3. The partitioning tells this processsor which elements to create

  dof_id_type num_elems = nx * ny * nz;
  const auto n_pieces = mesh.comm().size();
  const auto pid = mesh.comm().rank();

  std::unique_ptr<Elem> canonical_elem = libmesh_make_unique<T>();

  // Will get used to find the neighbors of an element
  std::vector<dof_id_type> neighbors(canonical_elem->n_neighbors());

  // Data structure that Metis will fill up on processor 0 and broadcast.
  std::vector<Metis::idx_t> part(num_elems);

  if (mesh.processor_id() == 0)
  {
    // Data structures and parameters needed only on processor 0 by Metis.
    // std::vector<Metis::idx_t> options(5);
    // Weight by the number of nodes
    std::vector<Metis::idx_t> vwgt(num_elems, canonical_elem->n_nodes());

    Metis::idx_t n = num_elems, // number of "nodes" (elements) in the graph
        nparts = n_pieces,      // number of subdomains to create
        edgecut = 0;            // the numbers of edges cut by the resulting partition

    METIS_CSR_Graph<Metis::idx_t> csr_graph;

    csr_graph.offsets.resize(num_elems + 1, 0);

    for (dof_id_type k = 0; k < nz; k++)
    {
      for (dof_id_type j = 0; j < ny; j++)
      {
        for (dof_id_type i = 0; i < nx; i++)
        {
          auto n_neighbors = num_neighbors<T>(nx, ny, nz, i, j, k);

          auto e_id = elem_id<T>(nx, ny, i, j, k);

          if (verbose)
            Moose::out << e_id << " num_neighbors: " << n_neighbors << std::endl;

          csr_graph.prep_n_nonzeros(e_id, n_neighbors);
        }
      }
    }

    csr_graph.prepare_for_use();

    if (verbose)
      for (auto offset : csr_graph.offsets)
        Moose::out << "offset: " << offset << std::endl;

    for (dof_id_type k = 0; k < nz; k++)
    {
      for (dof_id_type j = 0; j < ny; j++)
      {
        for (dof_id_type i = 0; i < nx; i++)
        {
          auto e_id = elem_id<T>(nx, ny, i, j, k);

          dof_id_type connection = 0;

          get_neighbors<T>(nx, ny, nz, i, j, k, neighbors);

          for (auto neighbor : neighbors)
          {
            if (neighbor != Elem::invalid_id)
            {
              if (verbose)
                Moose::out << e_id << ": " << connection << " = " << neighbor << std::endl;

              csr_graph(e_id, connection++) = neighbor;
            }
          }
        }
      }
    }

    if (n_pieces == 1)
    {
      // Just assign them all to proc 0
      for (auto & elem_pid : part)
        elem_pid = 0;
    }
    else
    {
      Metis::idx_t ncon = 1;

      // Use recursive if the number of partitions is less than or equal to 8
      if (n_pieces <= 8)
        Metis::METIS_PartGraphRecursive(&n,
                                        &ncon,
                                        &csr_graph.offsets[0],
                                        &csr_graph.vals[0],
                                        &vwgt[0],
                                        libmesh_nullptr,
                                        libmesh_nullptr,
                                        &nparts,
                                        libmesh_nullptr,
                                        libmesh_nullptr,
                                        libmesh_nullptr,
                                        &edgecut,
                                        &part[0]);

      // Otherwise  use kway
      else
        Metis::METIS_PartGraphKway(&n,
                                   &ncon,
                                   &csr_graph.offsets[0],
                                   &csr_graph.vals[0],
                                   &vwgt[0],
                                   libmesh_nullptr,
                                   libmesh_nullptr,
                                   &nparts,
                                   libmesh_nullptr,
                                   libmesh_nullptr,
                                   libmesh_nullptr,
                                   &edgecut,
                                   &part[0]);
    }
  } // end processor 0 part

  // Broadcast the resulting partition
  mesh.comm().broadcast(part);

  if (verbose)
    for (auto proc_id : part)
      Moose::out << "Part: " << proc_id << std::endl;

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Add elements this processor owns
  for (dof_id_type k = 0; k < nz; k++)
  {
    for (dof_id_type j = 0; j < ny; j++)
    {
      for (dof_id_type i = 0; i < nx; i++)
      {
        auto e_id = elem_id<Hex8>(nx, ny, i, j, k);

        if (static_cast<processor_id_type>(part[e_id]) == pid)
          add_element<T>(nx, ny, nz, i, j, k, e_id, pid, type, mesh, verbose);
      }
    }
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

  // Add the ghosts to the mesh
  for (auto & ghost_id : ghost_elems)
  {
    dof_id_type i = 0;
    dof_id_type j = 0;
    dof_id_type k = 0;

    get_indices<T>(nx, ny, ghost_id, i, j, k);

    add_element<T>(nx, ny, nz, i, j, k, ghost_id, part[ghost_id], type, mesh, verbose);
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

  // No need to renumber or find neighbors - done did it.
  // Avoid deprecation message/error by _also_ setting
  // allow_renumbering(false). This is a bit silly, but we want to
  // catch cases where people are purely using the old "skip"
  // interface and not the new flag setting one.
  mesh.allow_renumbering(false);
  mesh.prepare_for_use(/*skip_renumber (ignored!) = */ false,
                       /*skip_find_neighbors = */ true);

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
