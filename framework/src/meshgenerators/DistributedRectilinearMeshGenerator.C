//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedRectilinearMeshGenerator.h"
#include "CastUniquePointer.h"
#include "PetscExternalPartitioner.h"

#include "SerializerGuard.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/remote_elem.h"
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

  params.addParam<processor_id_type>(
      "num_cores_for_partition", 0, "Number of cores for partitioning the graph");

  params.addRangeCheckedParam<unsigned>(
      "num_side_layers",
      2,
      "num_side_layers>=1 & num_side_layers<5",
      "Number of layers of off-processor side neighbors is reserved during mesh generation");

  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  // Let this RM safeguard users specified ghosted layers
                                  rm_params.set<unsigned short>("layers") =
                                      obj_params.get<unsigned>("num_side_layers");
                                  // We can not attach geometric early here because some simulation
                                  // related info, such as, periodic BCs is not available yet during
                                  // an early stage. Periodic BCs will be passed into ghosting
                                  // functor during initFunctor of ElementSideNeighborLayers. There
                                  // is no hurt to attach geometric late for general simulations
                                  // that do not have extra requirements on ghosting elements. That
                                  // is especially true distributed generated meshes since there is
                                  // not much redundant info.
                                  rm_params.set<bool>("attach_geometric_early") = false;
                                });

  MooseEnum partition("graph linear square", "graph", false);
  params.addParam<MooseEnum>(
      "partition", partition, "Which method (graph linear square) use to partition mesh");

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

  params.addClassDescription(
      "Create a line, square, or cube mesh with uniformly spaced or biased elements.");

  return params;
}

DistributedRectilinearMeshGenerator::DistributedRectilinearMeshGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(declareMeshProperty("num_elements_x", getParam<dof_id_type>("nx"))),
    _ny(declareMeshProperty("num_elements_y", getParam<dof_id_type>("ny"))),
    _nz(declareMeshProperty("num_elements_z", getParam<dof_id_type>("nz"))),
    _xmin(declareMeshProperty("xmin", getParam<Real>("xmin"))),
    _xmax(declareMeshProperty("xmax", getParam<Real>("xmax"))),
    _ymin(declareMeshProperty("ymin", getParam<Real>("ymin"))),
    _ymax(declareMeshProperty("ymax", getParam<Real>("ymax"))),
    _zmin(declareMeshProperty("zmin", getParam<Real>("zmin"))),
    _zmax(declareMeshProperty("zmax", getParam<Real>("zmax"))),
    _num_cores_for_partition(getParam<processor_id_type>("num_cores_for_partition")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z")),
    _part_package(getParam<MooseEnum>("part_package")),
    _num_parts_per_compute_node(getParam<processor_id_type>("num_cores_per_compute_node")),
    _partition_method(getParam<MooseEnum>("partition")),
    _num_side_layers(getParam<unsigned>("num_side_layers"))
{
  declareMeshProperty("use_distributed_mesh", true);
}

template <>
dof_id_type
DistributedRectilinearMeshGenerator::numNeighbors<Edge2>(const dof_id_type nx,
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
void
DistributedRectilinearMeshGenerator::getNeighbors<Edge2>(const dof_id_type nx,
                                                         const dof_id_type /*ny*/,
                                                         const dof_id_type /*nz*/,
                                                         const dof_id_type i,
                                                         const dof_id_type /*j*/,
                                                         const dof_id_type /*k*/,
                                                         std::vector<dof_id_type> & neighbors,
                                                         const bool corner)

{
  if (corner)
  {
    // The elements on the opposite of the current boundary are required
    // for periodic boundary conditions
    neighbors[0] = (i - 1 + nx) % nx;
    neighbors[1] = (i + 1 + nx) % nx;

    return;
  }

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
void
DistributedRectilinearMeshGenerator::getIndices<Edge2>(const dof_id_type /*nx*/,
                                                       const dof_id_type /*ny*/,
                                                       const dof_id_type elem_id,
                                                       dof_id_type & i,
                                                       dof_id_type & /*j*/,
                                                       dof_id_type & /*k*/)
{
  i = elem_id;
}

template <>
void
DistributedRectilinearMeshGenerator::getGhostNeighbors<Edge2>(
    const dof_id_type nx,
    const dof_id_type /*ny*/,
    const dof_id_type /*nz*/,
    const MeshBase & mesh,
    const std::set<dof_id_type> & current_elems,
    std::set<dof_id_type> & ghost_elems)
{
  std::vector<dof_id_type> neighbors(2);

  for (auto elem_id : current_elems)
  {
    getNeighbors<Edge2>(nx, 0, 0, elem_id, 0, 0, neighbors, true);

    for (auto neighbor : neighbors)
      if (neighbor != Elem::invalid_id && !mesh.query_elem_ptr(neighbor))
        ghost_elems.insert(neighbor);
  }
}

template <>
dof_id_type
DistributedRectilinearMeshGenerator::elemId<Edge2>(const dof_id_type /*nx*/,
                                                   const dof_id_type /*ny*/,
                                                   const dof_id_type i,
                                                   const dof_id_type /*j*/,
                                                   const dof_id_type /*k*/)
{
  return i;
}

template <>
void
DistributedRectilinearMeshGenerator::addElement<Edge2>(const dof_id_type nx,
                                                       const dof_id_type /*ny*/,
                                                       const dof_id_type /*nz*/,
                                                       const dof_id_type /*i*/,
                                                       const dof_id_type /*j*/,
                                                       const dof_id_type /*k*/,
                                                       const dof_id_type elem_id,
                                                       const processor_id_type pid,
                                                       const ElemType /*type*/,
                                                       MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  auto node_offset = elem_id;

  Node * node0_ptr = mesh.query_node_ptr(node_offset);
  if (!node0_ptr)
  {
    std::unique_ptr<Node> new_node =
        Node::build(Point(static_cast<Real>(node_offset) / nx, 0, 0), node_offset);

    new_node->set_unique_id(nx + node_offset);
    new_node->processor_id() = pid;

    node0_ptr = mesh.add_node(std::move(new_node));
  }

  Node * node1_ptr = mesh.query_node_ptr(node_offset + 1);
  if (!node1_ptr)
  {
    std::unique_ptr<Node> new_node =
        Node::build(Point(static_cast<Real>(node_offset + 1) / nx, 0, 0), node_offset + 1);

    new_node->set_unique_id(nx + node_offset + 1);
    new_node->processor_id() = pid;

    node1_ptr = mesh.add_node(std::move(new_node));
  }

  Elem * elem = new Edge2;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  elem->set_unique_id(elem_id);
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
DistributedRectilinearMeshGenerator::setBoundaryNames<Edge2>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "left";
  boundary_info.sideset_name(1) = "right";
}

template <>
void
DistributedRectilinearMeshGenerator::scaleNodalPositions<Edge2>(dof_id_type /*nx*/,
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
dof_id_type
DistributedRectilinearMeshGenerator::numNeighbors<Quad4>(const dof_id_type nx,
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
dof_id_type
DistributedRectilinearMeshGenerator::elemId<Quad4>(const dof_id_type nx,
                                                   const dof_id_type /*nx*/,
                                                   const dof_id_type i,
                                                   const dof_id_type j,
                                                   const dof_id_type /*k*/)
{
  return (j * nx) + i;
}

template <>
void
DistributedRectilinearMeshGenerator::getNeighbors<Quad4>(const dof_id_type nx,
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
    // libMesh dof_id_type looks like unsigned int
    // We add one layer of point neighbors by default. Besides,
    // The elements on the opposite side of the current boundary are included
    // for, in case, periodic boundary conditions. The overhead is negligible
    // since you could consider every element has the same number of neighbors
    unsigned int nnb = 0;
    for (unsigned int ii = 0; ii <= 2; ii++)
      for (unsigned int jj = 0; jj <= 2; jj++)
        neighbors[nnb++] = elemId<Quad4>(nx, 0, (i + ii - 1 + nx) % nx, (j + jj - 1 + ny) % ny, 0);

    return;
  }
  // Bottom
  if (j != 0)
    neighbors[0] = elemId<Quad4>(nx, 0, i, j - 1, 0);

  // Right
  if (i != nx - 1)
    neighbors[1] = elemId<Quad4>(nx, 0, i + 1, j, 0);

  // Top
  if (j != ny - 1)
    neighbors[2] = elemId<Quad4>(nx, 0, i, j + 1, 0);

  // Left
  if (i != 0)
    neighbors[3] = elemId<Quad4>(nx, 0, i - 1, j, 0);
}

template <>
void
DistributedRectilinearMeshGenerator::getIndices<Quad4>(const dof_id_type nx,
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
void
DistributedRectilinearMeshGenerator::getGhostNeighbors<Quad4>(
    const dof_id_type nx,
    const dof_id_type ny,
    const dof_id_type /*nz*/,
    const MeshBase & mesh,
    const std::set<dof_id_type> & current_elems,
    std::set<dof_id_type> & ghost_elems)
{
  dof_id_type i, j, k;

  std::vector<dof_id_type> neighbors(9);

  for (auto elem_id : current_elems)
  {
    getIndices<Quad4>(nx, 0, elem_id, i, j, k);

    getNeighbors<Quad4>(nx, ny, 0, i, j, 0, neighbors, true);

    for (auto neighbor : neighbors)
      if (neighbor != Elem::invalid_id && !mesh.query_elem_ptr(neighbor))
        ghost_elems.insert(neighbor);
  }
}

template <>
dof_id_type
DistributedRectilinearMeshGenerator::nodeId<Quad4>(const ElemType /*type*/,
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
DistributedRectilinearMeshGenerator::addElement<Quad4>(const dof_id_type nx,
                                                       const dof_id_type ny,
                                                       const dof_id_type /*nz*/,
                                                       const dof_id_type i,
                                                       const dof_id_type j,
                                                       const dof_id_type /*k*/,
                                                       const dof_id_type elem_id,
                                                       const processor_id_type pid,
                                                       const ElemType type,
                                                       MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Bottom Left
  const dof_id_type node0_id = nodeId<Quad4>(type, nx, 0, i, j, 0);
  Node * node0_ptr = mesh.query_node_ptr(node0_id);
  if (!node0_ptr)
  {
    std::unique_ptr<Node> new_node =
        Node::build(Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, 0), node0_id);

    new_node->set_unique_id(nx * ny + node0_id);
    new_node->processor_id() = pid;

    node0_ptr = mesh.add_node(std::move(new_node));
  }

  // Bottom Right
  const dof_id_type node1_id = nodeId<Quad4>(type, nx, 0, i + 1, j, 0);
  Node * node1_ptr = mesh.query_node_ptr(node1_id);
  if (!node1_ptr)
  {
    std::unique_ptr<Node> new_node =
        Node::build(Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j) / ny, 0), node1_id);

    new_node->set_unique_id(nx * ny + node1_id);
    new_node->processor_id() = pid;

    node1_ptr = mesh.add_node(std::move(new_node));
  }

  // Top Right
  const dof_id_type node2_id = nodeId<Quad4>(type, nx, 0, i + 1, j + 1, 0);
  Node * node2_ptr = mesh.query_node_ptr(node2_id);
  if (!node2_ptr)
  {
    std::unique_ptr<Node> new_node = Node::build(
        Point(static_cast<Real>(i + 1) / nx, static_cast<Real>(j + 1) / ny, 0), node2_id);

    new_node->set_unique_id(nx * ny + node2_id);
    new_node->processor_id() = pid;

    node2_ptr = mesh.add_node(std::move(new_node));
  }

  // Top Left
  const dof_id_type node3_id = nodeId<Quad4>(type, nx, 0, i, j + 1, 0);
  Node * node3_ptr = mesh.query_node_ptr(node3_id);
  if (!node3_ptr)
  {
    std::unique_ptr<Node> new_node =
        Node::build(Point(static_cast<Real>(i) / nx, static_cast<Real>(j + 1) / ny, 0), node3_id);

    new_node->set_unique_id(nx * ny + node3_id);
    new_node->processor_id() = pid;

    node3_ptr = mesh.add_node(std::move(new_node));
  }

  Elem * elem = new Quad4;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  elem->set_unique_id(elem_id);
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
DistributedRectilinearMeshGenerator::setBoundaryNames<Quad4>(BoundaryInfo & boundary_info)
{
  boundary_info.sideset_name(0) = "bottom";
  boundary_info.sideset_name(1) = "right";
  boundary_info.sideset_name(2) = "top";
  boundary_info.sideset_name(3) = "left";
}

template <>
void
DistributedRectilinearMeshGenerator::scaleNodalPositions<Quad4>(dof_id_type /*nx*/,
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
dof_id_type
DistributedRectilinearMeshGenerator::elemId<Hex8>(const dof_id_type nx,
                                                  const dof_id_type ny,
                                                  const dof_id_type i,
                                                  const dof_id_type j,
                                                  const dof_id_type k)
{
  return i + (j * nx) + (k * nx * ny);
}

template <>
dof_id_type
DistributedRectilinearMeshGenerator::numNeighbors<Hex8>(const dof_id_type nx,
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
void
DistributedRectilinearMeshGenerator::getNeighbors<Hex8>(const dof_id_type nx,
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
    // We collect one layer of point neighbors
    // We add one layer of point neighbors by default. Besides,
    // The elements on the opposite side of the current boundary are included
    // for, in case, periodic boundary conditions. The overhead is negligible
    // since you could consider every element has the same number of neighbors
    unsigned int nnb = 0;
    for (unsigned int ii = 0; ii <= 2; ii++)
      for (unsigned int jj = 0; jj <= 2; jj++)
        for (unsigned int kk = 0; kk <= 2; kk++)
          neighbors[nnb++] = elemId<Hex8>(
              nx, ny, (i + ii - 1 + nx) % nx, (j + jj - 1 + ny) % ny, (k + kk - 1 + nz) % nz);

    return;
  }

  // Back
  if (k != 0)
    neighbors[0] = elemId<Hex8>(nx, ny, i, j, k - 1);

  // Bottom
  if (j != 0)
    neighbors[1] = elemId<Hex8>(nx, ny, i, j - 1, k);

  // Right
  if (i != nx - 1)
    neighbors[2] = elemId<Hex8>(nx, ny, i + 1, j, k);

  // Top
  if (j != ny - 1)
    neighbors[3] = elemId<Hex8>(nx, ny, i, j + 1, k);

  // Left
  if (i != 0)
    neighbors[4] = elemId<Hex8>(nx, ny, i - 1, j, k);

  // Front
  if (k != nz - 1)
    neighbors[5] = elemId<Hex8>(nx, ny, i, j, k + 1);
}

template <>
dof_id_type
DistributedRectilinearMeshGenerator::nodeId<Hex8>(const ElemType /*type*/,
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
DistributedRectilinearMeshGenerator::addPoint<Hex8>(const dof_id_type nx,
                                                    const dof_id_type ny,
                                                    const dof_id_type nz,
                                                    const dof_id_type i,
                                                    const dof_id_type j,
                                                    const dof_id_type k,
                                                    const ElemType type,
                                                    MeshBase & mesh)
{
  auto id = nodeId<Hex8>(type, nx, ny, i, j, k);
  Node * node_ptr = mesh.query_node_ptr(id);
  if (!node_ptr)
  {
    std::unique_ptr<Node> new_node = Node::build(
        Point(static_cast<Real>(i) / nx, static_cast<Real>(j) / ny, static_cast<Real>(k) / nz), id);

    new_node->set_unique_id(nx * ny * nz + id);

    node_ptr = mesh.add_node(std::move(new_node));
  }

  return node_ptr;
}

template <>
void
DistributedRectilinearMeshGenerator::addElement<Hex8>(const dof_id_type nx,
                                                      const dof_id_type ny,
                                                      const dof_id_type nz,
                                                      const dof_id_type i,
                                                      const dof_id_type j,
                                                      const dof_id_type k,
                                                      const dof_id_type elem_id,
                                                      const processor_id_type pid,
                                                      const ElemType type,
                                                      MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // This ordering was picked to match the ordering in mesh_generation.C
  auto node0_ptr = addPoint<Hex8>(nx, ny, nz, i, j, k, type, mesh);
  node0_ptr->processor_id() = pid;
  auto node1_ptr = addPoint<Hex8>(nx, ny, nz, i + 1, j, k, type, mesh);
  node1_ptr->processor_id() = pid;
  auto node2_ptr = addPoint<Hex8>(nx, ny, nz, i + 1, j + 1, k, type, mesh);
  node2_ptr->processor_id() = pid;
  auto node3_ptr = addPoint<Hex8>(nx, ny, nz, i, j + 1, k, type, mesh);
  node3_ptr->processor_id() = pid;
  auto node4_ptr = addPoint<Hex8>(nx, ny, nz, i, j, k + 1, type, mesh);
  node4_ptr->processor_id() = pid;
  auto node5_ptr = addPoint<Hex8>(nx, ny, nz, i + 1, j, k + 1, type, mesh);
  node5_ptr->processor_id() = pid;
  auto node6_ptr = addPoint<Hex8>(nx, ny, nz, i + 1, j + 1, k + 1, type, mesh);
  node6_ptr->processor_id() = pid;
  auto node7_ptr = addPoint<Hex8>(nx, ny, nz, i, j + 1, k + 1, type, mesh);
  node7_ptr->processor_id() = pid;

  Elem * elem = new Hex8;
  elem->set_id(elem_id);
  elem->processor_id() = pid;
  elem->set_unique_id(elem_id);
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
void
DistributedRectilinearMeshGenerator::getIndices<Hex8>(const dof_id_type nx,
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
void
DistributedRectilinearMeshGenerator::getGhostNeighbors<Hex8>(
    const dof_id_type nx,
    const dof_id_type ny,
    const dof_id_type nz,
    const MeshBase & mesh,
    const std::set<dof_id_type> & current_elems,
    std::set<dof_id_type> & ghost_elems)
{
  dof_id_type i, j, k;

  std::vector<dof_id_type> neighbors(27);

  for (auto elem_id : current_elems)
  {
    getIndices<Hex8>(nx, ny, elem_id, i, j, k);

    getNeighbors<Hex8>(nx, ny, nz, i, j, k, neighbors, true);

    for (auto neighbor : neighbors)
      if (neighbor != Elem::invalid_id && !mesh.query_elem_ptr(neighbor))
        ghost_elems.insert(neighbor);
  }
}

template <>
void
DistributedRectilinearMeshGenerator::setBoundaryNames<Hex8>(BoundaryInfo & boundary_info)
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
DistributedRectilinearMeshGenerator::scaleNodalPositions<Hex8>(dof_id_type /*nx*/,
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

template <>
void
DistributedRectilinearMeshGenerator::paritionSquarely<Edge2>(const dof_id_type nx,
                                                             const dof_id_type /*ny*/,
                                                             const dof_id_type /*nz*/,
                                                             const processor_id_type num_procs,
                                                             std::vector<dof_id_type> & istarts,
                                                             std::vector<dof_id_type> & jstarts,
                                                             std::vector<dof_id_type> & kstarts)
{
  // Starting indices along x direction
  istarts.resize(num_procs + 1);
  // Starting indices along y direction
  // There is only one processor along y direction since it is a 1D mesh
  jstarts.resize(2);
  jstarts[0] = 0;
  jstarts[1] = 1;
  // Starting indices along z direction
  // There is only one processor along z direction since it is a 1D mesh
  kstarts.resize(2);
  kstarts[0] = 0;
  kstarts[1] = 1;

  istarts[0] = 0;
  for (processor_id_type pid = 0; pid < num_procs; pid++)
    // Partition mesh evenly. The extra elements are assigned to the front processors
    istarts[pid + 1] = istarts[pid] + (nx / num_procs + ((nx % num_procs) > pid));
}

template <>
void
DistributedRectilinearMeshGenerator::paritionSquarely<Quad4>(const dof_id_type nx,
                                                             const dof_id_type ny,
                                                             const dof_id_type /*nz*/,
                                                             const processor_id_type num_procs,
                                                             std::vector<dof_id_type> & istarts,
                                                             std::vector<dof_id_type> & jstarts,
                                                             std::vector<dof_id_type> & kstarts)
{
  // Try for squarish distribution
  // The number of processors along x direction
  processor_id_type px =
      (processor_id_type)(0.5 + std::sqrt(((Real)nx) * ((Real)num_procs) / ((Real)ny)));

  if (!px)
    px = 1;

  // The number of processors along y direction
  processor_id_type py = 1;
  // Fact num_procs into px times py
  while (px > 0)
  {
    py = num_procs / px;
    if (py * px == num_procs)
      break;
    px--;
  }
  // More processors are needed for denser side
  if (nx > ny && py < px)
    std::swap(px, py);

  // Starting indices along x direction
  istarts.resize(px + 1);
  // Starting indices along y direction
  jstarts.resize(py + 1);
  // Starting indices along z direction
  kstarts.resize(2);
  // There is no elements along z direction
  kstarts[0] = 0;
  kstarts[1] = 1;

  istarts[0] = 0;
  for (processor_id_type pxid = 0; pxid < px; pxid++)
    // Partition elements evenly along x direction
    istarts[pxid + 1] = istarts[pxid] + nx / px + ((nx % px) > pxid);

  jstarts[0] = 0;
  for (processor_id_type pyid = 0; pyid < py; pyid++)
    // Partition elements evenly along y direction
    jstarts[pyid + 1] = jstarts[pyid] + (ny / py + ((ny % py) > pyid));
}

template <>
void
DistributedRectilinearMeshGenerator::paritionSquarely<Hex8>(const dof_id_type nx,
                                                            const dof_id_type ny,
                                                            const dof_id_type nz,
                                                            const processor_id_type num_procs,
                                                            std::vector<dof_id_type> & istarts,
                                                            std::vector<dof_id_type> & jstarts,
                                                            std::vector<dof_id_type> & kstarts)
{
  /* Try for squarish distribution */
  // The number of processors along y direction
  processor_id_type py =
      (processor_id_type)(0.5 + std::pow(((Real)ny * ny) * ((Real)num_procs) / ((Real)nz * nx),
                                         (Real)(1. / 3.)));
  if (!py)
    py = 1;
  // The number of processors for pxpz plane
  processor_id_type pxpz = 1;
  // Factorize num_procs into py times pxpz
  while (py > 0)
  {
    pxpz = num_procs / py;
    if (py * pxpz == num_procs)
      break;
    py--;
  }

  if (!py)
    py = 1;

  // There number of processors along x direction
  processor_id_type px =
      (processor_id_type)(0.5 + std::sqrt(((Real)nx) * ((Real)num_procs) / ((Real)nz * py)));
  if (!px)
    px = 1;
  // The number of processors along z direction
  processor_id_type pz = 1;
  // Factorize num_procs into px times py times pz
  while (px > 0)
  {
    pz = num_procs / (px * py);
    if (px * py * pz == num_procs)
      break;
    px--;
  }
  // denser mesh takes more processors
  if (nx > nz && px < pz)
    std::swap(px, pz);

  istarts.resize(px + 1);
  jstarts.resize(py + 1);
  kstarts.resize(pz + 1);

  istarts[0] = 0;
  for (processor_id_type pxid = 0; pxid < px; pxid++)
    // Partition mesh evenly along x direction
    istarts[pxid + 1] = istarts[pxid] + nx / px + ((nx % px) > pxid);

  jstarts[0] = 0;
  for (processor_id_type pyid = 0; pyid < py; pyid++)
    // Partition mesh evenly along y direction
    jstarts[pyid + 1] = jstarts[pyid] + (ny / py + ((ny % py) > pyid));

  kstarts[0] = 0;
  for (processor_id_type pzid = 0; pzid < pz; pzid++)
    // Partition mesh evenly along z direction
    kstarts[pzid + 1] = kstarts[pzid] + (nz / pz + ((nz % pz) > pzid));
}

template <typename T>
void
DistributedRectilinearMeshGenerator::buildCube(UnstructuredMesh & mesh,
                                               const unsigned int nx,
                                               unsigned int ny,
                                               unsigned int nz,
                                               const Real xmin,
                                               const Real xmax,
                                               const Real ymin,
                                               const Real ymax,
                                               const Real zmin,
                                               const Real zmax,
                                               const ElemType type)
{
  /// 1. "Partition" the element linearly (i.e. break them up into n_procs contiguous chunks
  /// 2. Create a (dual) graph of the local elements
  /// 3. Partition the graph using PetscExternalPartitioner
  /// 4. Push elements to new owners
  /// 5. Each processor creates only the elements it owns
  /// 6. Find the ghosts we need (all the elements that connect to at least one local mesh vertex)
  /// 7. Pull the PIDs of the ghosts
  /// 8. Add ghosts with the right PIDs to the mesh

  auto & comm = mesh.comm();

  dof_id_type num_elems = nx * ny * nz;

  const auto num_procs = comm.size();
  // Current processor ID
  const auto pid = comm.rank();

  if (_num_cores_for_partition > num_procs)
    mooseError("Number of cores for the graph partitioner is too large ", _num_cores_for_partition);

  if (!_num_cores_for_partition)
    _num_cores_for_partition = num_procs;

  auto & boundary_info = mesh.get_boundary_info();

  std::unique_ptr<Elem> canonical_elem = std::make_unique<T>();

  // Will get used to find the neighbors of an element
  std::vector<dof_id_type> neighbors(canonical_elem->n_neighbors());
  // Number of neighbors
  dof_id_type n_neighbors = canonical_elem->n_neighbors();

  // "Partition" the elements linearly across the processors
  dof_id_type num_local_elems;
  dof_id_type local_elems_begin;
  dof_id_type local_elems_end;
  if (pid < _num_cores_for_partition)
    MooseUtils::linearPartitionItems(num_elems,
                                     _num_cores_for_partition,
                                     pid,
                                     num_local_elems,
                                     local_elems_begin,
                                     local_elems_end);
  else
  {
    num_local_elems = 0;
    local_elems_begin = 0;
    local_elems_end = 0;
  }

  std::vector<std::vector<dof_id_type>> graph;

  // Fill in xadj and adjncy
  // xadj is the offset into adjncy
  // adjncy are the face neighbors of each element on this processor
  graph.resize(num_local_elems);
  // Build a distributed graph
  num_local_elems = 0;
  for (dof_id_type e_id = local_elems_begin; e_id < local_elems_end; e_id++)
  {
    dof_id_type i, j, k = 0;

    getIndices<T>(nx, ny, e_id, i, j, k);

    getNeighbors<T>(nx, ny, nz, i, j, k, neighbors, false);

    std::vector<dof_id_type> & row = graph[num_local_elems++];
    row.reserve(n_neighbors);

    for (auto neighbor : neighbors)
      if (neighbor != Elem::invalid_id)
        row.push_back(neighbor);
  }

  // Partition the distributed graph
  std::vector<dof_id_type> partition_vec;
  if (_partition_method == "linear")
  {
    mooseWarning(" LinearPartitioner is mainly used for setting up regression tests. For the "
                 "production run, please do not use it.");
    // The graph is already partitioned linearly via calling MooseUtils::linearPartitionItems
    partition_vec.resize(num_local_elems);
    // We use it as is
    std::fill(partition_vec.begin(), partition_vec.end(), pid);
  }
  else if (_partition_method == "square")
  {
    // Starting partition indices along x direction
    std::vector<dof_id_type> istarts;
    // Starting partition indices along y direction
    std::vector<dof_id_type> jstarts;
    // Starting partition indices along z direction
    std::vector<dof_id_type> kstarts;
    partition_vec.resize(num_local_elems);

    // Partition mesh evenly along each direction
    paritionSquarely<T>(nx, ny, nz, num_procs, istarts, jstarts, kstarts);
    // At least one processor
    mooseAssert(istarts.size() > 1, "At least there is one processor along x direction");
    processor_id_type px = istarts.size() - 1;
    // At least one processor
    mooseAssert(jstarts.size() > 1, "At least there is one processor along y direction");
    processor_id_type py = jstarts.size() - 1;
    // At least one processor
    mooseAssert(kstarts.size() > 1, "At least there is one processor along z direction");
    // processor_id_type pz = kstarts.size() -1;
    for (dof_id_type e_id = local_elems_begin; e_id < local_elems_end; e_id++)
    {
      dof_id_type i = 0, j = 0, k = 0;
      getIndices<T>(nx, ny, e_id, i, j, k);
      processor_id_type pi = 0, pj = 0, pk = 0;

      pi = (std::upper_bound(istarts.begin(), istarts.end(), i) - istarts.begin()) - 1;
      pj = (std::upper_bound(jstarts.begin(), jstarts.end(), j) - jstarts.begin()) - 1;
      pk = (std::upper_bound(kstarts.begin(), kstarts.end(), k) - kstarts.begin()) - 1;

      partition_vec[e_id - local_elems_begin] = pk * px * py + pj * px + pi;

      mooseAssert((pk * px * py + pj * px + pi) < num_procs, "processor id is too large");
    }
  }
  else if (_partition_method == "graph")
    PetscExternalPartitioner::partitionGraph(
        comm, graph, {}, {}, num_procs, _num_parts_per_compute_node, _part_package, partition_vec);
  else
    mooseError("Unsupported partition method " + _partition_method);

  mooseAssert(partition_vec.size() == num_local_elems, " Invalid partition was generateed ");

  // Use current elements to remote processors according to partition
  std::map<processor_id_type, std::vector<dof_id_type>> pushed_elements_vecs;

  for (dof_id_type e_id = local_elems_begin; e_id < local_elems_end; e_id++)
    pushed_elements_vecs[partition_vec[e_id - local_elems_begin]].push_back(e_id);

  // Collect new elements I should own
  std::vector<dof_id_type> my_new_elems;

  auto elements_action_functor =
      [&my_new_elems](processor_id_type /*pid*/, const std::vector<dof_id_type> & data)
  { std::copy(data.begin(), data.end(), std::back_inserter(my_new_elems)); };

  Parallel::push_parallel_vector_data(comm, pushed_elements_vecs, elements_action_functor);

  // Add the elements this processor owns
  for (auto e_id : my_new_elems)
  {
    dof_id_type i = 0, j = 0, k = 0;

    getIndices<T>(nx, ny, e_id, i, j, k);

    addElement<T>(nx, ny, nz, i, j, k, e_id, pid, type, mesh);
  }

  // Need to link up the local elements before we can know what's missing
  mesh.find_neighbors();

  // Get the ghosts (missing face neighbors)
  std::set<dof_id_type> ghost_elems;
  // Current local elements
  std::set<dof_id_type> current_elems;

  // Fill current elems
  // We will grow domain from current elements
  for (auto & elem_ptr : mesh.element_ptr_range())
    current_elems.insert(elem_ptr->id());

  // Grow domain layer by layer
  for (unsigned layer = 0; layer < _num_side_layers; layer++)
  {
    // getGhostNeighbors produces one layer of side neighbors
    getGhostNeighbors<T>(nx, ny, nz, mesh, current_elems, ghost_elems);
    // Merge ghost elements into current element list
    current_elems.insert(ghost_elems.begin(), ghost_elems.end());
  }
  // We do not need it anymore
  current_elems.clear();

  // Elements we're going to request from others
  std::map<processor_id_type, std::vector<dof_id_type>> ghost_elems_to_request;

  for (auto & ghost_id : ghost_elems)
  {
    // This is the processor ID the ghost_elem was originally assigned to
    auto proc_id = MooseUtils::linearPartitionChunk(num_elems, _num_cores_for_partition, ghost_id);

    // Using side-effect insertion on purpose
    ghost_elems_to_request[proc_id].push_back(ghost_id);
  }

  // Next set ghost object ids from other processors
  auto gather_functor =
      [local_elems_begin, partition_vec](processor_id_type /*pid*/,
                                         const std::vector<dof_id_type> & coming_ghost_elems,
                                         std::vector<dof_id_type> & pid_for_ghost_elems)
  {
    auto num_ghost_elems = coming_ghost_elems.size();
    pid_for_ghost_elems.resize(num_ghost_elems);

    dof_id_type num_local_elems = 0;

    for (auto elem : coming_ghost_elems)
      pid_for_ghost_elems[num_local_elems++] = partition_vec[elem - local_elems_begin];
  };

  std::unordered_map<dof_id_type, processor_id_type> ghost_elem_to_pid;

  auto action_functor =
      [&ghost_elem_to_pid](processor_id_type /*pid*/,
                           const std::vector<dof_id_type> & my_ghost_elems,
                           const std::vector<dof_id_type> & pid_for_my_ghost_elems)
  {
    dof_id_type num_local_elems = 0;

    for (auto elem : my_ghost_elems)
      ghost_elem_to_pid[elem] = pid_for_my_ghost_elems[num_local_elems++];
  };

  const dof_id_type * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm, ghost_elems_to_request, gather_functor, action_functor, ex);

  // Add the ghost elements to the mesh
  for (auto gtop : ghost_elem_to_pid)
  {
    auto ghost_id = gtop.first;
    auto proc_id = gtop.second;

    dof_id_type i = 0, j = 0, k = 0;

    getIndices<T>(nx, ny, ghost_id, i, j, k);

    addElement<T>(nx, ny, nz, i, j, k, ghost_id, proc_id, type, mesh);
  }

  mesh.find_neighbors(true);

  // Set RemoteElem neighbors
  for (auto & elem_ptr : mesh.element_ptr_range())
    for (unsigned int s = 0; s < elem_ptr->n_sides(); s++)
      if (!elem_ptr->neighbor_ptr(s) && !boundary_info.n_boundary_ids(elem_ptr, s))
        elem_ptr->set_neighbor(s, const_cast<RemoteElem *>(remote_elem));

  setBoundaryNames<T>(boundary_info);

  Partitioner::set_node_processor_ids(mesh);

  // Already partitioned!
  mesh.skip_partitioning(true);

  // Scale the nodal positions
  scaleNodalPositions<T>(nx, ny, nz, xmin, xmax, ymin, ymax, zmin, zmax, mesh);
}

std::unique_ptr<MeshBase>
DistributedRectilinearMeshGenerator::generate()
{
  // We will set up boundaries accordingly. We do not want to call
  // ghostGhostedBoundaries in which allgather_packed_range  is unscalable.
  // ghostGhostedBoundaries will gather all boundaries to every single processor
  _mesh->needGhostGhostedBoundaries(false);

  // DistributedRectilinearMeshGenerator always generates a distributed mesh
  auto mesh = buildDistributedMesh();

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
  // Let the mesh know that it's not serialized
  mesh->set_distributed();

  // Switching on MooseEnum
  switch (_dim)
  {
    // The build_XYZ mesh generation functions take an
    // UnstructuredMesh& as the first argument, hence the dynamic_cast.
    case 1:
      buildCube<Edge2>(
          dynamic_cast<UnstructuredMesh &>(*mesh), _nx, 1, 1, _xmin, _xmax, 0, 0, 0, 0, _elem_type);
      break;
    case 2:
      buildCube<Quad4>(dynamic_cast<UnstructuredMesh &>(*mesh),
                       _nx,
                       _ny,
                       1,
                       _xmin,
                       _xmax,
                       _ymin,
                       _ymax,
                       0,
                       0,
                       _elem_type);
      break;
    case 3:
      buildCube<Hex8>(dynamic_cast<UnstructuredMesh &>(*mesh),
                      _nx,
                      _ny,
                      _nz,
                      _xmin,
                      _xmax,
                      _ymin,
                      _ymax,
                      _zmin,
                      _zmax,
                      _elem_type);
      break;
    default:
      mooseError(
          getParam<MooseEnum>("elem_type"),
          " is not a currently supported element type for DistributedRectilinearMeshGenerator");
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

  // MeshOutput<MT>::write_equation_systems will automatically renumber node and element IDs.
  // So we have to make that consistent at the first place. Otherwise, the moose cached data such as
  // _block_node_list will be inconsistent when we doing MooseMesh::getNodeBlockIds. That being
  // said, moose will pass new ids to getNodeBlockIds while the cached _block_node_list still use
  // the old node IDs. Yes, you could say: go ahead to do a mesh update, but I would say no. I do
  // not change mesh and there is no point to update anything.
  mesh->allow_renumbering(true);
  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
