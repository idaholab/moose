//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatchSidesetGenerator.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "libmesh/hilbert_sfc_partitioner.h"
#include "libmesh/morton_sfc_partitioner.h"
#include "libmesh/enum_elem_type.h"

// libmesh elem types
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"

#include <set>
#include <limits>
#include "libmesh/mesh_tools.h"

registerMooseObject("HeatConductionApp", PatchSidesetGenerator);

InputParameters
PatchSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<BoundaryName>("boundary",
                                        "The boundary that will be divided into patches");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_patches", "n_patches>0", "Number of patches");

  MooseEnum partitioning = MooseMesh::partitioning(); // default MOOSE partitioning
  partitioning += "grid";                             // ...but also add our own
  params.addParam<MooseEnum>(
      "partitioner",
      partitioning,
      "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");

  MooseEnum direction("x y z radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction",
                             direction,
                             "Specifies the sort direction if using the centroid partitioner. "
                             "Available options: x, y, z, radial");

  params.addParamNamesToGroup("partitioner centroid_partitioner_direction", "Partitioning");

  params.addClassDescription(
      "Divides the given sideset into smaller patches of roughly equal size.");

  return params;
}

PatchSidesetGenerator::PatchSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _n_patches(getParam<unsigned int>("n_patches")),
    _sideset_name(getParam<BoundaryName>("boundary")),
    _partitioner_name(getParam<MooseEnum>("partitioner"))
{
}

std::unique_ptr<MeshBase>
PatchSidesetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  _mesh->errorIfDistributedMesh("PatchSidesetGenerator");

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // get dimensionality
  _dim = mesh->mesh_dimension() - 1;

  // get a list of all sides; vector of tuples (elem, loc_side, side_set)
  auto side_list = boundary_info.build_active_side_list();

  // check if the provided sideset name is actually a sideset id
  // if _sideset_name can be converted to integer it's interpreted
  // as sideset id
  std::stringstream ss;
  ss << _sideset_name;
  ss >> _sideset;
  if (ss.fail())
    _sideset = boundary_info.get_id_by_name(_sideset_name);

  // make sure that _sideset exists
  if (_sideset == BoundaryInfo::invalid_id)
    paramError("sideset_name", "Not a valid boundary");

  // create a dim - 1 dimensional mesh
  auto boundary_mesh = buildReplicatedMesh(mesh->mesh_dimension() - 1);
  boundary_mesh->set_mesh_dimension(mesh->mesh_dimension() - 1);
  boundary_mesh->set_spatial_dimension(mesh->mesh_dimension());

  // nodes in the new mesh by boundary_node_id (index)
  std::vector<Node *> boundary_nodes;
  // a map from the node numbering on the volumetric mesh to the numbering
  // on the boundary_mesh
  std::map<dof_id_type, dof_id_type> mesh_node_id_to_boundary_node_id;
  // a local counter keeping track of how many entries have been added to boundary_nodes
  dof_id_type boundary_node_id = 0;
  // a map from new element id in the boundary mesh to the element id/side/sideset
  // tuple it came from
  std::map<dof_id_type, std::tuple<dof_id_type, unsigned short int, boundary_id_type>>
      boundary_elem_to_mesh_elem;
  for (auto & side : side_list)
  {
    if (std::get<2>(side) == _sideset)
    {
      // the original volumetric mesh element
      const Elem * elem = mesh->elem_ptr(std::get<0>(side));

      // the boundary element
      std::unique_ptr<const Elem> boundary_elem = elem->side_ptr(std::get<1>(side));

      // an array that saves the boundary node ids of this elem in the right order
      std::vector<dof_id_type> bnd_elem_node_ids(boundary_elem->n_nodes());

      // loop through the nodes in boundary_elem
      for (MooseIndex(boundary_elem->n_nodes()) j = 0; j < boundary_elem->n_nodes(); ++j)
      {
        const Node * node = boundary_elem->node_ptr(j);

        // Is this node a new node?
        if (mesh_node_id_to_boundary_node_id.find(node->id()) ==
            mesh_node_id_to_boundary_node_id.end())
        {
          // yes, it is new, need to add it to the mesh_node_id_to_boundary_node_id map
          mesh_node_id_to_boundary_node_id.insert(
              std::pair<dof_id_type, dof_id_type>(node->id(), boundary_node_id));

          // this adds this node to the boundary mesh and puts it at the right position
          // in the boundary_nodes array
          Point pt(*node);
          boundary_nodes.push_back(boundary_mesh->add_point(pt, boundary_node_id));

          // keep track of the boundary node for setting up the element
          bnd_elem_node_ids[j] = boundary_node_id;

          // increment the boundary_node_id counter
          ++boundary_node_id;
        }
        else
          bnd_elem_node_ids[j] = mesh_node_id_to_boundary_node_id.find(node->id())->second;
      }

      // all nodes for this element have been added, so we can add the element to the
      // boundary mesh
      Elem * new_bnd_elem = boundaryElementHelper(*boundary_mesh, boundary_elem->type());

      // keep track of these new boundary elements in boundary_elem_to_mesh_elem
      boundary_elem_to_mesh_elem.insert(
          std::pair<dof_id_type, std::tuple<dof_id_type, unsigned short int, boundary_id_type>>(
              new_bnd_elem->id(), side));

      // set the nodes & subdomain_id of the new element by looping over the
      // boundary_elem and then inserting its nodes into new_bnd_elem in the
      // same order
      for (MooseIndex(boundary_elem->n_nodes()) j = 0; j < boundary_elem->n_nodes(); ++j)
      {
        dof_id_type old_node_id = boundary_elem->node_ptr(j)->id();
        if (mesh_node_id_to_boundary_node_id.find(old_node_id) ==
            mesh_node_id_to_boundary_node_id.end())
          mooseError("Node id", old_node_id, " not linked to new node id.");
        dof_id_type new_node_id = mesh_node_id_to_boundary_node_id.find(old_node_id)->second;
        new_bnd_elem->set_node(j) = boundary_nodes[new_node_id];
      }
    }
  }

  // partition the boundary mesh
  boundary_mesh->prepare_for_use();
  _n_boundary_mesh_elems = boundary_mesh->n_elem();
  if (_partitioner_name == "grid")
    partition(*boundary_mesh);
  else
  {
    auto partitioner_enum = getParam<MooseEnum>("partitioner");
    MooseMesh::setPartitioner(*boundary_mesh, partitioner_enum, false, _pars, *this);
    boundary_mesh->partition(_n_patches);
  }

  // make sure every partition has at least one element; if not rename and adjust _n_patches
  checkPartitionAndCompress(*boundary_mesh);

  // prepare sideset names and boundary_ids added to mesh
  std::vector<BoundaryName> sideset_names =
      sidesetNameHelper(boundary_info.get_sideset_name(_sideset));

  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, sideset_names, true);

  mooseAssert(sideset_names.size() == _n_patches,
              "sideset_names must have as many entries as user-requested number of patches.");
  mooseAssert(boundary_ids.size() == _n_patches,
              "boundary_ids must have as many entries as user-requested number of patches.");

  // loop through all elements in the boundary mesh and assign the side of
  // the _original_ element to the new sideset
  for (const auto & elem : boundary_mesh->active_element_ptr_range())
  {
    if (boundary_elem_to_mesh_elem.find(elem->id()) == boundary_elem_to_mesh_elem.end())
      mooseError("Element in the boundary mesh with id ",
                 elem->id(),
                 " not found in boundary_elem_to_mesh_elem.");

    auto side = boundary_elem_to_mesh_elem.find(elem->id())->second;

    mooseAssert(elem->processor_id() < boundary_ids.size(),
                "Processor id larger than number of patches.");
    boundary_info.add_side(
        std::get<0>(side), std::get<1>(side), boundary_ids[elem->processor_id()]);
  }

  // make sure new boundary names are set
  for (MooseIndex(boundary_ids.size()) j = 0; j < boundary_ids.size(); ++j)
  {
    boundary_info.sideset_name(boundary_ids[j]) = sideset_names[j];
    boundary_info.nodeset_name(boundary_ids[j]) = sideset_names[j];
  }

  return mesh;
}

void
PatchSidesetGenerator::partition(MeshBase & mesh)
{
  if (_partitioner_name == "grid")
  {
    // Figure out the physical bounds of the given mesh
    auto bounding_box = MeshTools::create_bounding_box(mesh);
    const auto & min = bounding_box.min();
    const auto & max = bounding_box.max();
    const auto & delta = max - min;

    // set number of elements
    std::vector<unsigned int> nelems(3);
    if (_dim == 1)
    {
      // find the largest component in delta
      unsigned int largest_id = 0;
      Real largest = delta(0);
      for (unsigned int j = 1; j < 3; ++j)
        if (largest < delta(j))
        {
          largest = delta(j);
          largest_id = j;
        }

      // set nelems now
      nelems = {1, 1, 1};
      nelems[largest_id] = _n_patches;
    }
    else
    {
      // find the smallest component in delta
      unsigned int smallest_id = 0;
      Real smallest = delta(0);
      for (unsigned int j = 1; j < 3; ++j)
        if (smallest > delta(j))
        {
          smallest = delta(j);
          smallest_id = j;
        }

      // store the ids for the two larger dimensions
      unsigned int id1 = 1, id2 = 2;
      if (smallest_id == 1)
        id1 = 0;
      else if (smallest_id == 2)
        id2 = 0;

      // set number of elements
      nelems[smallest_id] = 1;
      nelems[id1] = std::round(std::sqrt(delta(id1) / delta(id2) * _n_patches));
      nelems[id2] = std::round(std::sqrt(delta(id2) / delta(id1) * _n_patches));
      unsigned int final_n_patches = nelems[id1] * nelems[id2];
      // We need to check if the number of requested patches and the number of
      // actually created patches matches. If the two do not match, then a warning
      // is printed.
      if (_n_patches != final_n_patches)
      {
        _console << "Note: For creating radiation patches for boundary " << _sideset
                 << " using grid partitioner number of patches was changed from " << _n_patches
                 << " to " << final_n_patches << std::endl;
        _n_patches = final_n_patches;
      }
    }

    const Real dx = delta(0) / nelems[0];
    const Real dy = delta(1) / nelems[1];
    const Real dz = delta(2) / nelems[2];
    for (auto & elem_ptr : mesh.active_element_ptr_range())
    {
      const Point centroid = elem_ptr->vertex_average();
      processor_id_type proc_id;
      const unsigned int ix = std::floor((centroid(0) - min(0)) / dx);
      const unsigned int iy = std::floor((centroid(1) - min(1)) / dy);
      const unsigned int iz = std::floor((centroid(2) - min(2)) / dz);
      proc_id = ix + iy * nelems[0] + iz * nelems[0] * nelems[1];
      elem_ptr->processor_id() = proc_id;
    }
  }
  else
    mooseError("Partitioner ", _partitioner_name, " not recognized.");
}

void
PatchSidesetGenerator::checkPartitionAndCompress(MeshBase & mesh)
{
  std::set<processor_id_type> processor_ids;
  for (auto & elem_ptr : mesh.active_element_ptr_range())
    processor_ids.insert(elem_ptr->processor_id());

  if (processor_ids.size() == _n_patches)
    return;

  // at least one partition does not have an elem assigned to it
  // adjust _n_patches
  _console << "Some partitions for side set " << _sideset
           << " are empty. Adjusting number of patches from " << _n_patches << " to "
           << processor_ids.size() << std::endl;
  _n_patches = processor_ids.size();

  // create a vector and sort it
  std::vector<processor_id_type> processor_ids_vec;
  for (auto & p : processor_ids)
    processor_ids_vec.push_back(p);
  std::sort(processor_ids_vec.begin(), processor_ids_vec.end());

  // now remap the processor ids
  std::map<processor_id_type, processor_id_type> processor_id_remap;
  for (MooseIndex(processor_ids_vec.size()) j = 0; j < processor_ids_vec.size(); ++j)
    processor_id_remap[processor_ids_vec[j]] = j;

  for (auto & elem_ptr : mesh.active_element_ptr_range())
  {
    processor_id_type p = elem_ptr->processor_id();
    const auto & it = processor_id_remap.find(p);
    if (it == processor_id_remap.end())
      mooseError("Parition id ", p, " not in processor_id_remap.");
    elem_ptr->processor_id() = it->second;
  }
}

std::vector<BoundaryName>
PatchSidesetGenerator::sidesetNameHelper(const std::string & base_name) const
{
  std::vector<BoundaryName> rv;
  for (unsigned int j = 0; j < _n_patches; ++j)
  {
    std::stringstream ss;
    ss << base_name << "_" << j;
    rv.push_back(ss.str());
  }
  return rv;
}

Elem *
PatchSidesetGenerator::boundaryElementHelper(MeshBase & mesh, libMesh::ElemType type) const
{
  switch (type)
  {
    case 0:
      return mesh.add_elem(new libMesh::Edge2);
    case 1:
      return mesh.add_elem(new libMesh::Edge3);
    case 2:
      return mesh.add_elem(new libMesh::Edge4);
    case 3:
      return mesh.add_elem(new libMesh::Tri3);
    case 4:
      return mesh.add_elem(new libMesh::Tri6);
    case 5:
      return mesh.add_elem(new libMesh::Quad4);
    case 6:
      return mesh.add_elem(new libMesh::Quad8);
    case 7:
      return mesh.add_elem(new libMesh::Quad9);
    default:
      mooseError("Unsupported element type (libMesh elem_type enum): ", type);
  }
}
