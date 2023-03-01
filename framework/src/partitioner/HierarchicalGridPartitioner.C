//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HierarchicalGridPartitioner.h"

#include "GeneratedMesh.h"
#include "MooseApp.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/linear_partitioner.h"

registerMooseObject("MooseApp", HierarchicalGridPartitioner);

#include <memory>

InputParameters
HierarchicalGridPartitioner::validParams()
{
  auto params = MoosePartitioner::validParams();

  params.addRequiredRangeCheckedParam<unsigned int>(
      "nx_nodes", "nx_nodes > 0", "Number of compute nodes in the X direction");
  params.addRangeCheckedParam<unsigned int>(
      "ny_nodes", 0, "ny_nodes >= 0", "Number of compute nodes in the Y direction");
  params.addRangeCheckedParam<unsigned int>(
      "nz_nodes", 0, "nz_nodes >= 0", "Number of compute nodes in the Z direction");

  params.addRequiredRangeCheckedParam<unsigned int>(
      "nx_procs", "nx_procs > 0", "Number of processors in the X direction within each node");
  params.addRangeCheckedParam<unsigned int>(
      "ny_procs", 0, "ny_procs >= 0", "Number of processors in the Y direction within each node");
  params.addRangeCheckedParam<unsigned int>(
      "nz_procs", 0, "nz_procs >= 0", "Number of processors in the Z direction within each node");

  params.addClassDescription("Partitions a mesh into sub-partitions for each computational node"
                             " then into partitions within that node.  All partitions are made"
                             " using a regular grid.");

  return params;
}

HierarchicalGridPartitioner::HierarchicalGridPartitioner(const InputParameters & params)
  : MoosePartitioner(params),
    _mesh(*getCheckedPointerParam<MooseMesh *>("mesh")),
    _nx_nodes(getParam<unsigned int>("nx_nodes")),
    _ny_nodes(getParam<unsigned int>("ny_nodes")),
    _nz_nodes(getParam<unsigned int>("nz_nodes")),
    _nx_procs(getParam<unsigned int>("nx_procs")),
    _ny_procs(getParam<unsigned int>("ny_procs")),
    _nz_procs(getParam<unsigned int>("nz_procs"))
{
}

HierarchicalGridPartitioner::~HierarchicalGridPartitioner() {}

std::unique_ptr<Partitioner>
HierarchicalGridPartitioner::clone() const
{
  return std::make_unique<HierarchicalGridPartitioner>(_pars);
}

void
HierarchicalGridPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  const auto dim = mesh.spatial_dimension();
  if (_ny_procs == 0 && dim > 1)
    paramError("ny_procs", "Required for ", dim, "D meshes");
  if (_ny_nodes == 0 && dim > 1)
    paramError("ny_nodes", "Required for ", dim, "D meshes");
  if (_nz_procs == 0 && dim == 3)
    paramError("nz_procs", "Required for 3D meshes");
  if (_nz_nodes == 0 && dim == 3)
    paramError("nz_nodes", "Required for 3D meshes");

  auto total_nodes = _nx_nodes;

  if (mesh.spatial_dimension() >= 2)
    total_nodes *= _ny_nodes;

  if (mesh.spatial_dimension() == 3)
    total_nodes *= _nz_nodes;

  auto procs_per_node = _nx_procs;

  if (mesh.spatial_dimension() >= 2)
    procs_per_node *= _ny_procs;

  if (mesh.spatial_dimension() == 3)
    procs_per_node *= _nz_procs;

  if (procs_per_node * total_nodes != mesh.n_partitions())
    mooseError("Partitioning in ",
               name(),
               " doesn't add up to the total number of processors: ",
               procs_per_node * total_nodes);

  // Figure out the physical bounds of the given mesh
  auto nodes_bounding_box = MeshTools::create_bounding_box(mesh);
  const auto & nodes_min = nodes_bounding_box.min();
  const auto & nodes_max = nodes_bounding_box.max();

  // Bound the coarse mesh (n_nodes * n_nodes)
  auto nodes_mesh = std::make_unique<ReplicatedMesh>(this->_communicator);
  nodes_mesh->partitioner() = std::make_unique<LinearPartitioner>();

  if (mesh.spatial_dimension() == 2)
    MeshTools::Generation::build_cube(*nodes_mesh,
                                      _nx_nodes,
                                      _ny_nodes,
                                      _nz_nodes,
                                      nodes_min(0),
                                      nodes_max(0),
                                      nodes_min(1),
                                      nodes_max(1),
                                      nodes_min(2),
                                      nodes_max(2),
                                      QUAD4);
  else
    MeshTools::Generation::build_cube(*nodes_mesh,
                                      _nx_nodes,
                                      _ny_nodes,
                                      _nz_nodes,
                                      nodes_min(0),
                                      nodes_max(0),
                                      nodes_min(1),
                                      nodes_max(1),
                                      nodes_min(2),
                                      nodes_max(2),
                                      HEX8);

  // Now build the procs meshes
  std::vector<std::unique_ptr<ReplicatedMesh>> procs_meshes(nodes_mesh->n_elem());

  for (const auto & elem_ptr : nodes_mesh->active_element_ptr_range())
  {
    // Need to find the bounds of the elem
    Point min(std::numeric_limits<Real>::max(),
              std::numeric_limits<Real>::max(),
              std::numeric_limits<Real>::max());
    Point max(-std::numeric_limits<Real>::max(),
              -std::numeric_limits<Real>::max(),
              -std::numeric_limits<Real>::max());

    // Loop over all the nodes
    for (const auto & node : elem_ptr->node_ref_range())
    {
      min(0) = std::min(min(0), node(0));
      min(1) = std::min(min(1), node(1));
      min(2) = std::min(min(2), node(2));

      max(0) = std::max(max(0), node(0));
      max(1) = std::max(max(1), node(1));
      max(2) = std::max(max(2), node(2));
    }

    auto procs_mesh = std::make_unique<ReplicatedMesh>(this->_communicator);
    procs_mesh->partitioner() = std::make_unique<LinearPartitioner>();

    if (mesh.spatial_dimension() == 2)
      MeshTools::Generation::build_cube(*procs_mesh,
                                        _nx_procs,
                                        _ny_procs,
                                        _nz_procs,
                                        min(0),
                                        max(0),
                                        min(1),
                                        max(1),
                                        min(2),
                                        max(2),
                                        QUAD4);
    else
      MeshTools::Generation::build_cube(*procs_mesh,
                                        _nx_procs,
                                        _ny_procs,
                                        _nz_procs,
                                        min(0),
                                        max(0),
                                        min(1),
                                        max(1),
                                        min(2),
                                        max(2),
                                        HEX8);

    procs_meshes[elem_ptr->id()] = std::move(procs_mesh);
  }

  auto nodes_point_locator_ptr = nodes_mesh->sub_point_locator();

  std::vector<std::unique_ptr<PointLocatorBase>> procs_point_locators(procs_meshes.size());

  for (unsigned int i = 0; i < procs_meshes.size(); i++)
    procs_point_locators[i] = procs_meshes[i]->sub_point_locator();

  // Loop over all of the elements in the given mesh
  for (auto & elem_ptr : mesh.active_element_ptr_range())
  {
    auto elem_centroid = elem_ptr->vertex_average();

    // Find the element it lands in in the Nodes mesh
    auto nodes_elem_ptr = (*nodes_point_locator_ptr)(elem_centroid);

    auto nodes_elem_id = nodes_elem_ptr->id();

    // True if we found something
    if (nodes_elem_ptr)
    {
      // Now see where it lands within the procs mesh of that node
      auto procs_elem_ptr = (*procs_point_locators[nodes_elem_id])(elem_centroid);

      // Assign the _id_ of the cell to the processor_id (plus an offset for which node we're in)
      elem_ptr->processor_id() = procs_elem_ptr->id() + (nodes_elem_id * procs_per_node);
    }
    else // Should never happen (seriously - we create bounding boxes that should disallow this!)
      mooseError("HierarchicalGridPartitioner unable to locate element within the grid!");
  }
}
