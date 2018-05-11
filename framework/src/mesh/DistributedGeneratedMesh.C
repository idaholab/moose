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
  params.addParam<bool>(
      "gauss_lobatto_grid",
      false,
      "Grade mesh into boundaries according to Gauss-Lobatto quadrature spacing.");
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
    _gauss_lobatto_grid(getParam<bool>("gauss_lobatto_grid")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z"))
{
  if (_gauss_lobatto_grid && (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0))
    mooseError("Cannot apply both Gauss-Lobatto mesh grading and biasing at the same time.");

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
      distributed_build_line(dynamic_cast<UnstructuredMesh &>(mesh),
                             _nx,
                             _xmin,
                             _xmax,
                             _elem_type,
                             _gauss_lobatto_grid);
      break;
    case 2:
      build_square(dynamic_cast<UnstructuredMesh &>(mesh),
                   _nx,
                   _ny,
                   _xmin,
                   _xmax,
                   _ymin,
                   _ymax,
                   _elem_type,
                   _gauss_lobatto_grid);
      break;
    case 3:
      build_cube(dynamic_cast<UnstructuredMesh &>(getMesh()),
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
                 _gauss_lobatto_grid);
      break;
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

namespace
{
/**
 * Get the number of neighbors this element will have
 *
 * @param nx The number of elements in the mesh
 * @param i The index of this element
 * @return The number of neighboring elements
 */
inline dof_id_type
num_neighbors_edge(const dof_id_type nx, const dof_id_type i)
{
  // The ends only have one neighbor
  if (i == 0 || i == nx - 1)
    return 1;

  return 2;
}

/**
 * Get the IDs of the neighbors of a given element
 *
 * @param nx The number of elements in the mesh
 * @param i The index of this element
 * @param neighbors This will be filled with the IDs of the two neighbors or invalid_dof_id if there
 * is no neighbor.  THIS MUST be of size 2 BEFORE calling this function
 */
inline void
get_neighbors_edge(const dof_id_type nx, const dof_id_type i, std::vector<dof_id_type> & neighbors)
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

/**
 * Find the elements and sides that need ghost elements
 *
 * @param nx The number of elements in the mesh
 * @param mesh The mesh - without any ghost elements
 * @param ghost_elems The ghost elems that need to be added
 */
inline void
get_ghost_neighbors_edge(const dof_id_type nx,
                         const MeshBase & mesh,
                         std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.boundary_info;

  std::vector<dof_id_type> neighbors(2);

  for (auto elem_ptr : mesh.element_ptr_range())
  {
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
    {
      // No current neighbor
      if (!elem_ptr->neighbor_ptr(s))
      {
        // Not on a boundary
        if (!boundary_info->n_boundary_ids(elem_ptr, s))
        {
          get_neighbors_edge(nx, elem_ptr->id(), neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}

/**
 * Adds an edge element to the mesh
 */
void
add_element_edge(const dof_id_type nx,
                 const dof_id_type elem_id,
                 const processor_id_type pid,
                 const ElemType type,
                 MeshBase & mesh,
                 bool _verbose)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (_verbose)
    std::cout << "Adding elem: " << elem_id << " pid: " << pid << std::endl;

  switch (type)
  {
    case INVALID_ELEM:
    case EDGE2:
    {
      auto node_offset = elem_id;

      auto node0_ptr =
          mesh.add_point(Point(static_cast<Real>(node_offset) / nx, 0, 0), node_offset);
      node0_ptr->set_unique_id() = node_offset;

      auto node1_ptr =
          mesh.add_point(Point(static_cast<Real>(node_offset + 1) / nx, 0, 0), node_offset + 1);
      node1_ptr->set_unique_id() = node_offset + 1;

      if (_verbose)
        std::cout << "Adding elem: " << elem_id << std::endl;

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

      break;
    }

    case EDGE3:
    {
      auto node_offset = 2 * elem_id;

      auto node0_ptr =
          mesh.add_point(Point(static_cast<Real>(node_offset) / (2 * nx), 0, 0), node_offset);
      node0_ptr->set_unique_id() = node_offset;

      auto node1_ptr = mesh.add_point(Point(static_cast<Real>(node_offset + 1) / (2 * nx), 0, 0),
                                      node_offset + 1);
      node1_ptr->set_unique_id() = node_offset + 1;

      auto node2_ptr = mesh.add_point(Point(static_cast<Real>(node_offset + 2) / (2 * nx), 0, 0),
                                      node_offset + 2);
      node2_ptr->set_unique_id() = node_offset + 2;

      Elem * elem = new Edge3;
      elem->set_id(elem_id);
      elem->processor_id() = pid;
      //      elem->set_unique_id() = elem_id;
      elem = mesh.add_elem(elem);
      elem->set_node(0) = node0_ptr;
      elem->set_node(2) = node2_ptr;
      elem->set_node(1) = node1_ptr;

      if (elem_id == 0)
        boundary_info.add_side(elem, 0, 0);

      if (elem_id == nx - 1)
        boundary_info.add_side(elem, 1, 1);

      break;
    }

    case EDGE4:
    {
      auto node_offset = 3 * elem_id;

      auto node0_ptr =
          mesh.add_point(Point(static_cast<Real>(node_offset) / (3 * nx), 0, 0), node_offset);
      node0_ptr->set_unique_id() = node_offset;

      auto node1_ptr = mesh.add_point(Point(static_cast<Real>(node_offset + 1) / (3 * nx), 0, 0),
                                      node_offset + 1);
      node1_ptr->set_unique_id() = node_offset + 1;

      auto node2_ptr = mesh.add_point(Point(static_cast<Real>(node_offset + 2) / (3 * nx), 0, 0),
                                      node_offset + 2);
      node2_ptr->set_unique_id() = node_offset + 2;

      auto node3_ptr = mesh.add_point(Point(static_cast<Real>(node_offset + 3) / (3 * nx), 0, 0),
                                      node_offset + 3);
      node3_ptr->set_unique_id() = node_offset + 3;

      Elem * elem = new Edge3;
      elem->set_id(elem_id);
      elem->processor_id() = pid;
      //      elem->set_unique_id() = elem_id;
      elem = mesh.add_elem(elem);
      elem->set_node(0) = node0_ptr;
      elem->set_node(2) = node2_ptr;
      elem->set_node(3) = node3_ptr;
      elem->set_node(1) = node1_ptr;

      if (elem_id == 0)
        boundary_info.add_side(elem, 0, 0);

      if (elem_id == nx - 1)
        boundary_info.add_side(elem, 1, 1);

      break;
    }

    default:
      libmesh_error_msg("ERROR: Unrecognized 1D element type.");
  }
}

/**
 * Get the number of neighbors this element will have
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @return The number of neighboring elements
 */
inline dof_id_type
num_neighbors_quad(const dof_id_type nx,
                   const dof_id_type ny,
                   const dof_id_type i,
                   const dof_id_type j)
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

/**
 * Get the number of neighbors this element will have
 *
 * @param nx The number of elements in the x direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @return The ID of the i,j element
 */
inline dof_id_type
elem_id_quad(const dof_id_type nx, const dof_id_type i, const dof_id_type j)
{
  return (j * nx) + i;
}

/**
 * Get the IDs of the neighbors of a given element
 *
 * @param nx The number of elements in the x direction
 * @param nx The number of elements in the y direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @param neighbors This will be filled with the IDs of the two neighbors or invalid_dof_id if there
 * is no neighbor.  THIS MUST be of size 4 BEFORE calling this function
 */
inline void
get_neighbors_quad(const dof_id_type nx,
                   const dof_id_type ny,
                   const dof_id_type i,
                   const dof_id_type j,
                   std::vector<dof_id_type> & neighbors)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

  // Bottom
  if (j != 0)
    neighbors[0] = elem_id_quad(nx, i, j - 1);

  // Right
  if (i != nx - 1)
    neighbors[1] = elem_id_quad(nx, i + 1, j);

  // Top
  if (j != ny - 1)
    neighbors[2] = elem_id_quad(nx, i, j + 1);

  // Left
  if (i != 0)
    neighbors[3] = elem_id_quad(nx, i - 1, j);
}

/**
 * Compute the i,j indices of a given element ID
 *
 * @param elem_id The ID of the element
 * @param i Output: The index in the x direction
 * @param j Output: The index in the y direction
 */
inline void
get_indices_quad(const dof_id_type nx, const dof_id_type elem_id, dof_id_type & i, dof_id_type & j)
{
  i = elem_id % nx;
  j = (elem_id - i) / nx;
}

/**
 * Find the elements and sides that need ghost elements
 *
 * @param nx The number of elements in the mesh
 * @param mesh The mesh - without any ghost elements
 * @param ghost_elems The ghost elems that need to be added
 */
inline void
get_ghost_neighbors_quad(const dof_id_type nx,
                         const dof_id_type ny,
                         const MeshBase & mesh,
                         std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.boundary_info;

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
        if (!boundary_info->n_boundary_ids(elem_ptr, s))
        {
          auto elem_id = elem_ptr->id();

          get_indices_quad(nx, elem_id, i, j);

          get_neighbors_quad(nx, ny, i, j, neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}

/**
 * A useful inline function which replaces the macros
 * used previously.  Not private since this is a namespace,
 * but would be if this were a class.  The first one returns
 * the proper node number for 2D elements while the second
 * one returns the node number for 3D elements.
 */
inline dof_id_type
node_id_2d(const ElemType type, const dof_id_type nx, const dof_id_type i, const dof_id_type j)
{
  switch (type)
  {
    case INVALID_ELEM:
    case QUAD4:
    case TRI3:
    {
      return i + j * (nx + 1);
    }

    case QUAD8:
    case QUAD9:
    case TRI6:
    {
      return i + j * (2 * nx + 1);
    }

    default:
      libmesh_error_msg("ERROR: Unrecognized 2D element type.");
  }

  return libMesh::invalid_uint;
}

/**
 * Adds a quad element to the mesh
 */
void
add_element_quad(const dof_id_type nx,
                 const dof_id_type ny,
                 const dof_id_type i,
                 const dof_id_type j,
                 const dof_id_type elem_id,
                 const processor_id_type pid,
                 const ElemType type,
                 MeshBase & mesh,
                 bool _verbose)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (_verbose)
    std::cout << "Adding elem: " << elem_id << " pid: " << pid << std::endl;

  switch (type)
  {
    case INVALID_ELEM:
    case QUAD4:
    {
      // Bottom Left
      auto node0_ptr =
          mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0),
                         node_id_2d(type, nx, i, j));
      node0_ptr->set_unique_id() = node_id_2d(type, nx, i, j);

      // Bottom Right
      auto node1_ptr =
          mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j) / ny, 0),
                         node_id_2d(type, nx, i + 1, j));
      node1_ptr->set_unique_id() = node_id_2d(type, nx, i + 1, j);

      // Top Right
      auto node2_ptr =
          mesh.add_point(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j + 1) / ny, 0),
                         node_id_2d(type, nx, i + 1, j + 1));
      node2_ptr->set_unique_id() = node_id_2d(type, nx, i + 1, j + 1);

      // Top Left
      auto node3_ptr =
          mesh.add_point(Point(static_cast<Real>(i) / nx, static_cast<Real>(j + 1) / ny, 0),
                         node_id_2d(type, nx, i, j + 1));
      node3_ptr->set_unique_id() = node_id_2d(type, nx, i, j + 1);

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

      break;
    }
    default:
      mooseError("This element type is not supported by DistributedGeneratedMesh yet.");
  }
}

/**
 * Get the IDs of the neighbors of a given element
 *
 * @param nx The number of elements in the x direction
 * @param nx The number of elements in the y direction
 * @param i The x index of this element
 * @param j The y index of this element
 * @param neighbors This will be filled with the IDs of the two neighbors or invalid_dof_id if there
 * is no neighbor.  THIS MUST be of size 6 BEFORE calling this function
 */
inline void
get_neighbors_hex(const dof_id_type nx,
                  const dof_id_type ny,
                  const dof_id_type i,
                  const dof_id_type j,
                  std::vector<dof_id_type> & neighbors)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

  // Bottom
  if (j != 0)
    neighbors[0] = elem_id_quad(nx, i, j - 1);

  // Right
  if (i != nx - 1)
    neighbors[1] = elem_id_quad(nx, i + 1, j);

  // Top
  if (j != ny - 1)
    neighbors[2] = elem_id_quad(nx, i, j + 1);

  // Left
  if (i != 0)
    neighbors[3] = elem_id_quad(nx, i - 1, j);
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
inline dof_id_type
num_neighbors_hex(const dof_id_type nx,
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
inline dof_id_type
elem_id_hex(const dof_id_type nx,
            const dof_id_type ny,
            const dof_id_type i,
            const dof_id_type j,
            const dof_id_type k)
{
  return i + (j * nx) + (k * nx * ny);
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
inline void
get_neighbors_hex(const dof_id_type nx,
                  const dof_id_type ny,
                  const dof_id_type nz,
                  const dof_id_type i,
                  const dof_id_type j,
                  const dof_id_type k,
                  std::vector<dof_id_type> & neighbors)
{
  std::fill(neighbors.begin(), neighbors.end(), Elem::invalid_id);

  // Bottom
  if (j != 0)
    neighbors[0] = elem_id_hex(nx, ny, i, j - 1, k);

  // Right
  if (i != nx - 1)
    neighbors[1] = elem_id_hex(nx, ny, i + 1, j, k);

  // Top
  if (j != ny - 1)
    neighbors[2] = elem_id_hex(nx, ny, i, j + 1, k);

  // Left
  if (i != 0)
    neighbors[3] = elem_id_hex(nx, ny, i - 1, j, k);

  // Back
  if (k != 0)
    neighbors[4] = elem_id_hex(nx, ny, i, j, k - 1);

  // Front
  if (k != nz - 1)
    neighbors[5] = elem_id_hex(nx, ny, i, j, k + 1);
}

// Same as the function above, but for 3D elements
inline dof_id_type
node_id_3d(const ElemType type,
           const dof_id_type nx,
           const dof_id_type ny,
           const dof_id_type i,
           const dof_id_type j,
           const dof_id_type k)
{
  switch (type)
  {
    case INVALID_ELEM:
    case HEX8:
    case PRISM6:
    {
      return i + (nx + 1) * (j + k * (ny + 1));
    }

    case HEX20:
    case HEX27:
    case TET4:     // TET4's are created from an initial HEX27 discretization
    case TET10:    // TET10's are created from an initial HEX27 discretization
    case PYRAMID5: // PYRAMID5's are created from an initial HEX27 discretization
    case PYRAMID13:
    case PYRAMID14:
    case PRISM15:
    case PRISM18:
    {
      return i + (2 * nx + 1) * (j + k * (2 * ny + 1));
    }

    default:
      libmesh_error_msg("ERROR: Unrecognized element type.");
  }

  return libMesh::invalid_uint;
}

Node *
add_point_hex(const dof_id_type nx,
              const dof_id_type ny,
              const dof_id_type nz,
              const dof_id_type i,
              const dof_id_type j,
              const dof_id_type k,
              const ElemType type,
              MeshBase & mesh)
{
  auto id = node_id_3d(type, nx, ny, i, j, k);
  auto node_ptr = mesh.add_point(
      Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, static_cast<Real>(k) / nz), id);
  node_ptr->set_unique_id() = id;

  return node_ptr;
}

/**
 * Adds a hex element to the mesh
 */
void
add_element_hex(const dof_id_type nx,
                const dof_id_type ny,
                const dof_id_type nz,
                const dof_id_type i,
                const dof_id_type j,
                const dof_id_type k,
                const dof_id_type elem_id,
                const processor_id_type pid,
                const ElemType type,
                MeshBase & mesh,
                bool _verbose)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (_verbose)
    std::cout << "Adding elem: " << elem_id << " pid: " << pid << std::endl;

  switch (type)
  {
    case INVALID_ELEM:
    case HEX8:
    {
      // This ordering was picked to match the ordering in mesh_generation.C
      auto node0_ptr = add_point_hex(nx, ny, nz, i, j, k, type, mesh);
      auto node1_ptr = add_point_hex(nx, ny, nz, i + 1, j, k, type, mesh);
      auto node2_ptr = add_point_hex(nx, ny, nz, i + 1, j + 1, k, type, mesh);
      auto node3_ptr = add_point_hex(nx, ny, nz, i, j + 1, k, type, mesh);
      auto node4_ptr = add_point_hex(nx, ny, nz, i, j, k + 1, type, mesh);
      auto node5_ptr = add_point_hex(nx, ny, nz, i + 1, j, k + 1, type, mesh);
      auto node6_ptr = add_point_hex(nx, ny, nz, i + 1, j + 1, k + 1, type, mesh);
      auto node7_ptr = add_point_hex(nx, ny, nz, i, j + 1, k + 1, type, mesh);

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

      break;
    }
    default:
      mooseError("This element type is not supported by DistributedGeneratedMesh yet.");
  }
}

/**
 * Find the elements and sides that need ghost elements
 *
 * @param nx The number of elements in the x direction
 * @param ny The number of elements in the y direction
 * @param mesh The mesh - without any ghost elements
 * @param ghost_elems The ghost elems that need to be added
 */
inline void
get_ghost_neighbors_hex(const dof_id_type nx,
                        const dof_id_type ny,
                        const MeshBase & mesh,
                        std::set<dof_id_type> & ghost_elems)
{
  auto & boundary_info = mesh.boundary_info;

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
        if (!boundary_info->n_boundary_ids(elem_ptr, s))
        {
          auto elem_id = elem_ptr->id();

          get_indices_hex(nx, ny, elem_id, i, j, k);

          get_neighbors_hex(nx, ny, nz, i, j, k, neighbors);

          ghost_elems.insert(neighbors[s]);
        }
      }
    }
  }
}
}

void
DistributedGeneratedMesh::distributed_build_line(UnstructuredMesh & mesh,
                                                 const dof_id_type nx,
                                                 const Real xmin,
                                                 const Real xmax,
                                                 const ElemType type,
                                                 const bool /*gauss_lobatto_grid*/)
{
  dof_id_type num_elems = nx;
  const auto n_pieces = comm().size();
  const auto pid = comm().rank();

  std::unique_ptr<Elem> canonical_elem = nullptr;

  // Will get used to find the neighbors of an element
  // MUST be of size _2_.
  std::vector<dof_id_type> neighbors(2);

  // Build 1 of the element we want to build so we can query stuff about it
  switch (type)
  {
    case EDGE2:
      canonical_elem = libmesh_make_unique<Edge2>();
      break;
    case EDGE3:
      canonical_elem = libmesh_make_unique<Edge3>();
      break;
    case EDGE4:
      canonical_elem = libmesh_make_unique<Edge3>();
      break;
    default:
      mooseError("Invalid ElemType for distributed_build_line in DistributedGeneratedMesh ",
                 name());
  }

  // Data structure that Metis will fill up on processor 0 and broadcast.
  std::vector<Metis::idx_t> part(num_elems);

  if (mesh.processor_id() == 0)
  {
    // Data structures and parameters needed only on processor 0 by Metis.
    // std::vector<Metis::idx_t> options(5);
    // Weight by the number of nodes
    std::vector<Metis::idx_t> vwgt(num_elems, canonical_elem->n_nodes());

    Metis::idx_t n = static_cast<Metis::idx_t>(
                     num_elems), // number of "nodes" (elements) in the graph
        // wgtflag = 2,                                // weights on vertices only, none on edges
        // numflag = 0,                                // C-style 0-based numbering
        nparts = static_cast<Metis::idx_t>(n_pieces), // number of subdomains to create
        edgecut = 0; // the numbers of edges cut by the resulting partition

    METIS_CSR_Graph<Metis::idx_t> csr_graph;

    csr_graph.offsets.resize(num_elems + 1, 0);

    for (dof_id_type elem_id = 0; elem_id < num_elems; elem_id++)
    {
      auto num_neighbors = num_neighbors_edge(nx, elem_id);

      if (_verbose)
        std::cout << "num_neighbors: " << num_neighbors << std::endl;

      csr_graph.prep_n_nonzeros(elem_id, num_neighbors);
    }

    csr_graph.prepare_for_use();

    if (_verbose)
      for (auto offset : csr_graph.offsets)
        std::cout << "offset: " << offset << std::endl;

    for (dof_id_type elem_id = 0; elem_id < num_elems; elem_id++)
    {
      dof_id_type connection = 0;

      get_neighbors_edge(nx, elem_id, neighbors);

      for (auto neighbor : neighbors)
        if (neighbor != Elem::invalid_id)
        {
          if (_verbose)
            std::cout << elem_id << ": " << connection << " = " << neighbor << std::endl;

          csr_graph(elem_id, connection++) = neighbor;
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

  ////// Now build the mesh

  // Note!  No reason to "reserve_nodes" for DistributedMesh!

  // For 1D, node ids are directly indexed from element IDs

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Add elements this processor owns
  for (dof_id_type elem_id = 0; elem_id < num_elems; elem_id++)
    if (static_cast<processor_id_type>(part[elem_id]) == pid)
      add_element_edge(nx, elem_id, pid, type, mesh, _verbose);

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Need to link up the local elements before we can know what's missing
  mesh.find_neighbors();

  if (_verbose)
    std::cout << "After first find_neighbors" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Get the ghosts (missing face neighbors)
  std::set<dof_id_type> ghost_elems;
  get_ghost_neighbors_edge(nx, mesh, ghost_elems);

  // Add the ghosts to the mesh
  for (auto & ghost_id : ghost_elems)
    add_element_edge(nx, ghost_id, part[ghost_id], type, mesh, _verbose);

  if (_verbose)
    std::cout << "After adding ghosts" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  mesh.find_neighbors(true);

  if (_verbose)
    std::cout << "After second find neighbors " << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  for (auto & elem_ptr : mesh.element_ptr_range())
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
      if (!elem_ptr->neighbor(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
        elem_ptr->set_neighbor(s, const_cast<RemoteElem *>(remote_elem));

  if (_verbose)
    std::cout << "After adding remote elements" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  boundary_info.sideset_name(0) = "left";
  boundary_info.sideset_name(1) = "right";

  Partitioner::set_node_processor_ids(mesh);

  if (_verbose)
    std::cout << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  if (_verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      std::cout << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  // Already partitioned!
  mesh.skip_partitioning(true);

  //  mesh.update_post_partitioning();
  //  MeshCommunication().make_elems_parallel_consistent(mesh);
  //  MeshCommunication().make_node_unique_ids_parallel_consistent(mesh);

  //  std::cout << "Getting ready to renumber" << std::endl;
  //  mesh.renumber_nodes_and_elements();
  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      std::cout << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id()
                << " uid: " << elem_ptr->unique_id() << std::endl;

  if (_verbose)
    std::cout << "Getting ready to prepare for use" << std::endl;

  mesh.prepare_for_use(true, true); // No need to renumber or find neighbors - done did it.

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      std::cout << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id() << std::endl;

  if (_verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      std::cout << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  if (_verbose)
    std::cout << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  // Scale the nodal positions
  for (auto & node_ptr : mesh.node_ptr_range())
    (*node_ptr)(0) = (*node_ptr)(0) * (xmax - xmin) + xmin;

  if (_verbose)
    mesh.print_info();
}

void
DistributedGeneratedMesh::build_square(UnstructuredMesh & mesh,
                                       const unsigned int nx,
                                       const unsigned int ny,
                                       const Real xmin,
                                       const Real xmax,
                                       const Real ymin,
                                       const Real ymax,
                                       const ElemType type,
                                       const bool /*gauss_lobatto_grid*/)
{
  dof_id_type num_elems = nx * ny;
  const auto n_pieces = comm().size();
  const auto pid = comm().rank();

  std::unique_ptr<Elem> canonical_elem = nullptr;

  // Will get used to find the neighbors of an element
  // MUST be of size _2_.
  std::vector<dof_id_type> neighbors(4);

  // Build 1 of the element we want to build so we can query stuff about it
  switch (type)
  {
    case QUAD4:
      canonical_elem = libmesh_make_unique<Quad4>();
      break;
    default:
      mooseError("Invalid ElemType for distributed_build_line in DistributedGeneratedMesh ",
                 name());
  }

  // Data structure that Metis will fill up on processor 0 and broadcast.
  std::vector<Metis::idx_t> part(num_elems);

  if (mesh.processor_id() == 0)
  {
    // Data structures and parameters needed only on processor 0 by Metis.
    // std::vector<Metis::idx_t> options(5);
    // Weight by the number of nodes
    std::vector<Metis::idx_t> vwgt(num_elems, canonical_elem->n_nodes());

    Metis::idx_t n = static_cast<Metis::idx_t>(
                     num_elems), // number of "nodes" (elements) in the graph
        // wgtflag = 2,                                // weights on vertices only, none on edges
        // numflag = 0,                                // C-style 0-based numbering
        nparts = static_cast<Metis::idx_t>(n_pieces), // number of subdomains to create
        edgecut = 0; // the numbers of edges cut by the resulting partition

    METIS_CSR_Graph<Metis::idx_t> csr_graph;

    csr_graph.offsets.resize(num_elems + 1, 0);

    for (dof_id_type j = 0; j < ny; j++)
    {
      for (dof_id_type i = 0; i < nx; i++)
      {
        auto num_neighbors = num_neighbors_quad(nx, ny, i, j);

        auto elem_id = elem_id_quad(nx, i, j);

        if (_verbose)
          std::cout << "num_neighbors: " << num_neighbors << std::endl;

        csr_graph.prep_n_nonzeros(elem_id, num_neighbors);
      }
    }

    csr_graph.prepare_for_use();

    if (_verbose)
      for (auto offset : csr_graph.offsets)
        std::cout << "offset: " << offset << std::endl;

    for (dof_id_type j = 0; j < ny; j++)
    {
      for (dof_id_type i = 0; i < nx; i++)
      {
        auto elem_id = elem_id_quad(nx, i, j);

        dof_id_type connection = 0;

        get_neighbors_quad(nx, ny, i, j, neighbors);

        for (auto neighbor : neighbors)
        {
          if (neighbor != Elem::invalid_id)
          {
            if (_verbose)
              std::cout << elem_id << ": " << connection << " = " << neighbor << std::endl;

            csr_graph(elem_id, connection++) = neighbor;
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

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Add elements this processor owns
  for (dof_id_type j = 0; j < ny; j++)
  {
    for (dof_id_type i = 0; i < nx; i++)
    {
      auto elem_id = elem_id_quad(nx, i, j);

      if (static_cast<processor_id_type>(part[elem_id]) == pid)
        add_element_quad(nx, ny, i, j, elem_id, pid, type, mesh, _verbose);
    }
  }

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Need to link up the local elements before we can know what's missing
  mesh.find_neighbors();

  if (_verbose)
    std::cout << "After first find_neighbors" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Get the ghosts (missing face neighbors)
  std::set<dof_id_type> ghost_elems;
  get_ghost_neighbors_quad(nx, ny, mesh, ghost_elems);

  // Add the ghosts to the mesh
  for (auto & ghost_id : ghost_elems)
  {
    dof_id_type i, j;

    get_indices_quad(nx, ghost_id, i, j);

    add_element_quad(nx, ny, i, j, ghost_id, part[ghost_id], type, mesh, _verbose);
  }

  if (_verbose)
    std::cout << "After adding ghosts" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  mesh.find_neighbors(true);

  if (_verbose)
    std::cout << "After second find neighbors " << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Set RemoteElem neighbors
  for (auto & elem_ptr : mesh.element_ptr_range())
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
      if (!elem_ptr->neighbor(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
        elem_ptr->set_neighbor(s, const_cast<RemoteElem *>(remote_elem));

  if (_verbose)
    std::cout << "After adding remote elements" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "right";
  boundary_info.sideset_name(2) = "top";
  boundary_info.sideset_name(3) = "left";

  Partitioner::set_node_processor_ids(mesh);

  if (_verbose)
    std::cout << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  if (_verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      std::cout << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  // Already partitioned!
  mesh.skip_partitioning(true);

  //  mesh.update_post_partitioning();
  //  MeshCommunication().make_elems_parallel_consistent(mesh);
  //  MeshCommunication().make_node_unique_ids_parallel_consistent(mesh);

  //  std::cout << "Getting ready to renumber" << std::endl;
  //  mesh.renumber_nodes_and_elements();

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      std::cout << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id()
                << " uid: " << elem_ptr->unique_id() << std::endl;

  if (_verbose)
    std::cout << "Getting ready to prepare for use" << std::endl;

  mesh.prepare_for_use(true, true); // No need to renumber or find neighbors - done did it.

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      std::cout << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id() << std::endl;

  if (_verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      std::cout << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  if (_verbose)
    std::cout << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  // Scale the nodal positions
  for (auto & node_ptr : mesh.node_ptr_range())
  {
    (*node_ptr)(0) = (*node_ptr)(0) * (xmax - xmin) + xmin;
    (*node_ptr)(1) = (*node_ptr)(1) * (ymax - ymin) + ymin;
  }

  if (_verbose)
    mesh.print_info();
}

void
DistributedGeneratedMesh::build_cube(UnstructuredMesh & mesh,
                                     const unsigned int nx,
                                     const unsigned int ny,
                                     const unsigned int nz,
                                     const Real xmin,
                                     const Real xmax,
                                     const Real ymin,
                                     const Real ymax,
                                     const Real zmin,
                                     const Real zmax,
                                     const ElemType type,
                                     const bool gauss_lobatto_grid)
{
  dof_id_type num_elems = nx * ny * nz;
  const auto n_pieces = comm().size();
  const auto pid = comm().rank();

  std::unique_ptr<Elem> canonical_elem = nullptr;

  // Will get used to find the neighbors of an element
  // MUST be of size _2_.
  std::vector<dof_id_type> neighbors(6);

  // Build 1 of the element we want to build so we can query stuff about it
  switch (type)
  {
    case HEX8:
      canonical_elem = libmesh_make_unique<Hex8>();
      break;
    default:
      mooseError("Invalid ElemType for distributed_build_line in DistributedGeneratedMesh ",
                 name());
  }

  // Data structure that Metis will fill up on processor 0 and broadcast.
  std::vector<Metis::idx_t> part(num_elems);

  if (mesh.processor_id() == 0)
  {
    // Data structures and parameters needed only on processor 0 by Metis.
    // std::vector<Metis::idx_t> options(5);
    // Weight by the number of nodes
    std::vector<Metis::idx_t> vwgt(num_elems, canonical_elem->n_nodes());

    Metis::idx_t n = static_cast<Metis::idx_t>(
                     num_elems), // number of "nodes" (elements) in the graph
        // wgtflag = 2,                                // weights on vertices only, none on edges
        // numflag = 0,                                // C-style 0-based numbering
        nparts = static_cast<Metis::idx_t>(n_pieces), // number of subdomains to create
        edgecut = 0; // the numbers of edges cut by the resulting partition

    METIS_CSR_Graph<Metis::idx_t> csr_graph;

    csr_graph.offsets.resize(num_elems + 1, 0);

    for (dof_id_type k = 0; k < nz; k++)
    {
      for (dof_id_type j = 0; j < ny; j++)
      {
        for (dof_id_type i = 0; i < nx; i++)
        {
          auto num_neighbors = num_neighbors_hex(nx, ny, nz, i, j, k);

          auto elem_id = elem_id_hex(nx, ny, i, j, k);

          if (_verbose)
            std::cout << elem_id << " num_neighbors: " << num_neighbors << std::endl;

          csr_graph.prep_n_nonzeros(elem_id, num_neighbors);
        }
      }
    }

    csr_graph.prepare_for_use();

    if (_verbose)
      for (auto offset : csr_graph.offsets)
        std::cout << "offset: " << offset << std::endl;

    for (dof_id_type k = 0; k < nz; k++)
    {
      for (dof_id_type j = 0; j < ny; j++)
      {
        for (dof_id_type i = 0; i < nx; i++)
        {
          auto elem_id = elem_id_hex(nx, ny, i, j, k);

          dof_id_type connection = 0;

          get_neighbors_hex(nx, ny, nz, i, j, k, neighbors);

          for (auto neighbor : neighbors)
          {
            if (neighbor != Elem::invalid_id)
            {
              if (_verbose)
                std::cout << elem_id << ": " << connection << " = " << neighbor << std::endl;

              csr_graph(elem_id, connection++) = neighbor;
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

  if (_verbose)
    for (auto proc_id : part)
      std::cout << "Part: " << proc_id << std::endl;

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Add elements this processor owns
  for (dof_id_type k = 0; k < nz; k++)
  {
    for (dof_id_type j = 0; j < ny; j++)
    {
      for (dof_id_type i = 0; i < nx; i++)
      {
        auto elem_id = elem_id_hex(nx, ny, i, j, k);

        if (static_cast<processor_id_type>(part[elem_id]) == pid)
          add_element_hex(nx, ny, nz, i, j, k, elem_id, pid, type, mesh, _verbose);
      }
    }
  }

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Need to link up the local elements before we can know what's missing
  mesh.find_neighbors();

  if (_verbose)
    std::cout << "After first find_neighbors" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  /*

  // Get the ghosts (missing face neighbors)
  std::set<dof_id_type> ghost_elems;
  get_ghost_neighbors_quad(nx, ny, mesh, ghost_elems);

  // Add the ghosts to the mesh
  for (auto & ghost_id : ghost_elems)
  {
    dof_id_type i, j;

    get_indices_quad(nx, ghost_id, i, j);

    add_element_quad(nx, ny, i, j, ghost_id, part[ghost_id], type, mesh, _verbose);
  }

  if (_verbose)
    std::cout << "After adding ghosts" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  mesh.find_neighbors(true);

  if (_verbose)
    std::cout << "After second find neighbors " << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  // Set RemoteElem neighbors
  for (auto & elem_ptr : mesh.element_ptr_range())
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
      if (!elem_ptr->neighbor(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
        elem_ptr->set_neighbor(s, const_cast<RemoteElem *>(remote_elem));

  if (_verbose)
    std::cout << "After adding remote elements" << std::endl;

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
        std::cout << "Elem neighbor: " << elem_ptr->neighbor_ptr(s) << " is remote "
                  << (elem_ptr->neighbor_ptr(s) == remote_elem) << std::endl;

  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "right";
  boundary_info.sideset_name(2) = "top";
  boundary_info.sideset_name(3) = "left";

  Partitioner::set_node_processor_ids(mesh);

  if (_verbose)
    std::cout << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  if (_verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      std::cout << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  // Already partitioned!
  mesh.skip_partitioning(true);

  //  mesh.update_post_partitioning();
  //  MeshCommunication().make_elems_parallel_consistent(mesh);
  //  MeshCommunication().make_node_unique_ids_parallel_consistent(mesh);

  //  std::cout << "Getting ready to renumber" << std::endl;
  //  mesh.renumber_nodes_and_elements();

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      std::cout << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id()
                << " uid: " << elem_ptr->unique_id() << std::endl;

  if (_verbose)
    std::cout << "Getting ready to prepare for use" << std::endl;

  mesh.prepare_for_use(true, true); // No need to renumber or find neighbors - done did it.

  if (_verbose)
    for (auto & elem_ptr : mesh.element_ptr_range())
      std::cout << "Elem: " << elem_ptr->id() << " pid: " << elem_ptr->processor_id() << std::endl;

  if (_verbose)
    for (auto & node_ptr : mesh.node_ptr_range())
      std::cout << node_ptr->id() << ":" << node_ptr->processor_id() << std::endl;

  if (_verbose)
    std::cout << "mesh dim: " << mesh.mesh_dimension() << std::endl;

  // Scale the nodal positions
  for (auto & node_ptr : mesh.node_ptr_range())
  {
    (*node_ptr)(0) = (*node_ptr)(0) * (xmax - xmin) + xmin;
    (*node_ptr)(1) = (*node_ptr)(1) * (ymax - ymin) + ymin;
  }

  if (_verbose)
    mesh.print_info();
  */
}
