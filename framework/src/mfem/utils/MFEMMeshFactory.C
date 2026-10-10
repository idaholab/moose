//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMeshFactory.h"
#include "LibmeshMFEMBlockInfo.h"
#include "LibmeshMFEMMesh.h"
#include "MFEMMesh.h"
#include "MooseMesh.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <tuple>
#include <vector>

namespace
{
using libMesh::boundary_id_type;
using libMesh::dof_id_type;
using libMesh::subdomain_id_type;
using ElementIDsForBlockID = LibmeshMFEMMesh::ElementIDsForBlockID;
using NodeIDsForElementID = LibmeshMFEMMesh::NodeIDsForElementID;
using NodeIDsForBoundaryID = LibmeshMFEMMesh::NodeIDsForBoundaryID;
using SideIDsForBoundaryID = LibmeshMFEMMesh::SideIDsForBoundaryID;
using BlockIDsForBoundaryID = LibmeshMFEMMesh::BlockIDsForBoundaryID;
using CoordinatesForNodeID = LibmeshMFEMMesh::CoordinatesForNodeID;
/// libMesh IDs of the elements with a side on each boundary
using ElementIDsForBoundaryID = std::map<boundary_id_type, std::vector<dof_id_type>>;

/**
 * An internal method used to create maps from each boundary ID to vectors of side IDs
 * and element IDs.
 */
std::tuple<ElementIDsForBoundaryID, SideIDsForBoundaryID>
buildBoundaryInfo(const MeshBase & libmesh)
{
  ElementIDsForBoundaryID element_ids_for_boundary_id;
  SideIDsForBoundaryID side_ids_for_boundary_id;

  // Read the sides on each boundary directly from libMesh, rather than from the MooseMesh
  // boundary element cache, which would keep pointers to elements that are deleted when a
  // temporarily serialized distributed mesh is distributed again.
  for (const auto & [element_id, side_id, boundary_id] :
       libmesh.get_boundary_info().build_active_side_list())
  {
    element_ids_for_boundary_id[boundary_id].push_back(element_id);
    side_ids_for_boundary_id[boundary_id].push_back(side_id);
  }

  return {element_ids_for_boundary_id, side_ids_for_boundary_id};
}

/**
 * Create a mapping from each boundary ID to a vector of vectors containing the global node
 * IDs of nodes that lie on the faces of elements that fall on the boundary.
 */
NodeIDsForBoundaryID
buildBoundaryNodeIDs(const MooseMesh & mesh,
                     const std::vector<boundary_id_type> & unique_side_bound_ids,
                     const ElementIDsForBoundaryID & element_ids_for_bound,
                     const SideIDsForBoundaryID & side_ids_for_bound)
{
  NodeIDsForBoundaryID node_ids_for_bound_id;

  // Iterate over all bound IDs.
  for (const auto bound_id : unique_side_bound_ids)
  {
    // Get element IDs of element on bound (and their sides that are on bound).
    auto & bound_element_ids = libmesh_map_find(element_ids_for_bound, bound_id);
    auto & bound_element_sides = libmesh_map_find(side_ids_for_bound, bound_id);

    // Create vector to store the node ids of all bound nodes.
    std::vector<std::vector<dof_id_type>> bound_node_ids(bound_element_ids.size());

    // Iterate over elements on bound.
    for (const auto jelement : index_range(bound_element_ids))
    {
      // Get element ID and the bound side.
      const auto bound_element_global_id = bound_element_ids[jelement];
      const auto bound_element_side = bound_element_sides[jelement];

      const Elem * element_ptr = mesh.elemPtr(bound_element_global_id);

      // Get vector of local node IDs on bound side of element, and convert them to global IDs.
      const auto local_node_ids = element_ptr->nodes_on_side(bound_element_side);
      auto & global_node_ids = bound_node_ids[jelement];
      global_node_ids.reserve(local_node_ids.size());
      for (const auto local_node_id : local_node_ids)
        global_node_ids.push_back(element_ptr->node_id(local_node_id));
    }

    // Add to the map.
    node_ids_for_bound_id[bound_id] = std::move(bound_node_ids);
  }

  return node_ids_for_bound_id;
}

/**
 * Builds two maps:
 * 1. Mapping from each block ID --> vector containing all element IDs for block.
 * 2. Mapping from each element --> vector containing all global node IDs for element.
 */
std::tuple<ElementIDsForBlockID, NodeIDsForElementID>
buildElementAndNodeIDs(MeshBase & libmesh,
                       const LibmeshMFEMBlockInfo & block_info,
                       const std::vector<subdomain_id_type> & unique_block_ids)
{
  ElementIDsForBlockID element_ids_for_block_id;
  NodeIDsForElementID node_ids_for_element_id;

  for (const auto block_id : unique_block_ids)
  {
    auto & element_info = block_info.blockElement(block_id);

    std::vector<dof_id_type> elements_in_block;

    auto active_block_elements_begin = libmesh.active_subdomain_elements_begin(block_id);
    auto active_block_elements_end = libmesh.active_subdomain_elements_end(block_id);

    for (auto element_iterator = active_block_elements_begin;
         element_iterator != active_block_elements_end;
         element_iterator++)
    {
      auto element_ptr = *element_iterator;

      const auto element_id = element_ptr->id();

      std::vector<dof_id_type> element_node_ids(element_info.num_nodes);

      elements_in_block.push_back(element_id);

      for (const auto node_counter : make_range(element_info.num_nodes))
      {
        element_node_ids[node_counter] = element_ptr->node_id(node_counter);
      }

      node_ids_for_element_id[element_id] = std::move(element_node_ids);
    }

    elements_in_block.shrink_to_fit();

    // Add to map.
    element_ids_for_block_id[block_id] = std::move(elements_in_block);
  }

  return {element_ids_for_block_id, node_ids_for_element_id};
}

/**
 * Iterates through each block to find the elements in the block. For each
 * element in a block, it runs through the nodes in the block and adds only
 * the corner nodes to a vector. This is then sorted and only unique global
 * node IDs are retained.
 */
std::vector<dof_id_type>
buildUniqueCornerNodeIDs(const LibmeshMFEMBlockInfo & block_info,
                         const std::vector<subdomain_id_type> & unique_block_ids,
                         const ElementIDsForBlockID & element_ids_for_block_id,
                         const NodeIDsForElementID & node_ids_for_element_id)
{
  std::vector<dof_id_type> unique_corner_node_ids;

  // Iterate through all nodes (on edge of each element) and add their global IDs
  // to the unique_corner_node_ids vector.
  for (const auto block_id : unique_block_ids)
  {
    auto & block_element = block_info.blockElement(block_id);
    auto & element_ids = libmesh_map_find(element_ids_for_block_id, block_id);
    for (const auto element_id : element_ids)
    {
      auto & node_ids = libmesh_map_find(node_ids_for_element_id, element_id);

      // Only use the nodes on the edge of the element!
      for (const auto knode : make_range(block_element.num_corner_nodes))
      {
        unique_corner_node_ids.push_back(node_ids[knode]);
      }
    }
  }

  // Sort unique_vertex_ids in ascending order and remove duplicate node IDs.
  std::sort(unique_corner_node_ids.begin(), unique_corner_node_ids.end());

  auto new_end = std::unique(unique_corner_node_ids.begin(), unique_corner_node_ids.end());

  unique_corner_node_ids.resize(std::distance(unique_corner_node_ids.begin(), new_end));

  return unique_corner_node_ids;
}

/**
 * Assemble information on block elements .
 */
LibmeshMFEMBlockInfo
buildBlockInfo(MeshBase & libmesh,
               const std::vector<subdomain_id_type> & unique_block_ids,
               bool fallback,
               bool first_order)
{
  LibmeshMFEMBlockInfo block_info(libmesh.mesh_dimension(), fallback, first_order);
  /**
   * Iterate over the block_ids. Note that we only need to extract the first element from
   * each block since only a single element type can be specified per block.
   */
  for (const auto block_id : unique_block_ids)
  {
    auto element_range = libmesh.active_subdomain_elements_ptr_range(block_id);
    if (element_range.begin() == element_range.end())
    {
      mooseError("Block '", block_id, "' contains no elements.");
    }

    auto first_element_ptr = *element_range.begin();

    block_info.addBlockElement(
        block_id, first_element_ptr->type(), first_element_ptr->mapping_type());
  }
  return block_info;
}

/**
 * Blocks/subdomains are separate subsets of the mesh that could have different
 * material properties etc. This method returns a vector containing the unique
 * IDs of each block in the mesh. This will be passed to the MFEMMesh constructor
 * which sets the attribute of each element to the ID of the block that it is a
 * part of.
 */
std::vector<subdomain_id_type>
getLibmeshBlockIDs(const MeshBase & libmesh)
{
  // Identify all subdomains (blocks) in the entire mesh (global == true).
  std::set<subdomain_id_type> block_ids_set;
  libmesh.subdomain_ids(block_ids_set, true);

  return {block_ids_set.begin(), block_ids_set.end()};
}

/**
 * Returns a vector containing the IDs of all boundaries.
 */
std::vector<boundary_id_type>
getSideBoundaryIDs(const MeshBase & libmesh)
{
  const libMesh::BoundaryInfo & boundary_info = libmesh.get_boundary_info();
  const std::set<boundary_id_type> & side_boundary_ids_set = boundary_info.get_side_boundary_ids();

  std::vector<boundary_id_type> side_boundary_ids(side_boundary_ids_set.size());

  int counter = 0;
  for (auto side_boundary_id : side_boundary_ids_set)
  {
    // MFEM boundary attributes must be positive.
    if (side_boundary_id <= 0)
      mooseError("Boundary ID ",
                 side_boundary_id,
                 " can not be represented in an MFEM mesh, which requires positive boundary IDs. "
                 "Renumber the boundary, for example with a RenameBoundaryGenerator.");
    side_boundary_ids[counter++] = side_boundary_id;
  }

  std::sort(side_boundary_ids.begin(), side_boundary_ids.end());

  return side_boundary_ids;
}

/**
 * Maps from the element ID to the block ID.
 */
std::map<dof_id_type, subdomain_id_type>
getBlockIDForElementID(const ElementIDsForBlockID & element_ids_for_block_id)
{
  std::map<dof_id_type, subdomain_id_type> block_id_for_element_id;

  for (const auto & key_value : element_ids_for_block_id)
  {
    auto block_id = key_value.first;
    auto & element_ids = key_value.second;

    for (const auto & element_id : element_ids)
    {
      block_id_for_element_id[element_id] = block_id;
    }
  }

  return block_id_for_element_id;
}

/**
 * Maps from the boundary ID to a vector containing the block IDs of all elements that lie on
 * the boundary.
 */
BlockIDsForBoundaryID
getBlockIDsForBoundaryID(const ElementIDsForBlockID & element_ids_for_block_id,
                         const ElementIDsForBoundaryID & element_ids_for_boundary_id)
{
  auto block_id_for_element_id = getBlockIDForElementID(element_ids_for_block_id);

  BlockIDsForBoundaryID block_ids_for_boundary_id;

  for (const auto & key_value : element_ids_for_boundary_id)
  {
    auto boundary_id = key_value.first;
    auto & element_ids = key_value.second;

    std::vector<subdomain_id_type> block_ids(element_ids.size());

    int ielement = 0;
    for (const auto & element_id : element_ids)
    {
      block_ids[ielement++] = libmesh_map_find(block_id_for_element_id, element_id);
    }

    block_ids_for_boundary_id[boundary_id] = std::move(block_ids);
  }

  return block_ids_for_boundary_id;
}

/**
 * Returns the libMesh partitioning, giving the rank owning each element in the order the
 * elements are added to the MFEM mesh, which is block by block.
 */
std::vector<int>
getMeshPartitioning(const MeshBase & libmesh,
                    const std::vector<subdomain_id_type> & unique_block_ids,
                    const ElementIDsForBlockID & element_ids_for_block_id)
{
  std::vector<int> partitioning;
  partitioning.reserve(libmesh.n_active_elem());
  for (const auto block_id : unique_block_ids)
    for (const auto element_id : libmesh_map_find(element_ids_for_block_id, block_id))
      partitioning.push_back(libmesh.elem_ref(element_id).processor_id());
  return partitioning;
}
}

namespace Moose::MFEM
{
std::shared_ptr<mfem::ParMesh>
buildMFEMMesh(MooseMesh & mesh, bool fallback, bool first_order)
{
  // If working with an MFEMMesh object then the underlying
  // `mfem::ParMesh` is already available.
  if (auto * mfem_mesh = dynamic_cast<MFEMMesh *>(&mesh))
  {
    return mfem_mesh->getMFEMParMeshPtr();
  }
  // Otherwise we must construct it ourselves. Start by building a serial mesh.

  // 1. If the mesh is distributed, gather the nodes and elements onto each processor for the
  // duration of the conversion.
  libMesh::MeshSerializer serializer(mesh.getMesh());

  // 2. Get the unique libmesh IDs of each block in the mesh.
  const auto unique_block_ids = getLibmeshBlockIDs(mesh.getMesh());
  std::map<libMesh::subdomain_id_type, std::string> block_ids_to_names =
      mesh.getMesh().get_subdomain_name_map();

  // 3. Retrieve information about the elements used within the mesh.
  LibmeshMFEMBlockInfo block_info =
      buildBlockInfo(mesh.getMesh(), unique_block_ids, fallback, first_order);

  // 4. Build maps:
  // Map from block ID --> vector of element IDs.
  // Map from element ID --> vector of global node IDs.
  auto [element_ids_for_block_id, node_ids_for_element_id] =
      buildElementAndNodeIDs(mesh.getMesh(), block_info, unique_block_ids);

  // 5. Create vector containing the IDs of all nodes that are on the corners of
  // elements. MFEM only requires the corner nodes.
  const auto unique_corner_node_ids = buildUniqueCornerNodeIDs(
      block_info, unique_block_ids, element_ids_for_block_id, node_ids_for_element_id);

  // 6. Create a map to hold the x, y, z coordinates for each unique node.
  CoordinatesForNodeID coordinates_for_node_id;

  for (auto node_ptr : mesh.getMesh().node_ptr_range())
  {
    auto & node = *node_ptr;

    std::array<double, 3> coordinates = {node(0), node(1), node(2)};

    coordinates_for_node_id[node.id()] = std::move(coordinates);
  }

  // 7.
  // element_ids_for_boundary_id stores the ids of each element on each boundary.
  // side_ids_for_boundary_id stores the sides of those elements that are on each boundary.
  auto [element_ids_for_boundary_id, side_ids_for_boundary_id] = buildBoundaryInfo(mesh.getMesh());

  // 8. Get a vector containing the IDs of all side boundaries in the mesh.
  const auto unique_side_boundary_ids = getSideBoundaryIDs(mesh.getMesh());
  std::map<libMesh::boundary_id_type, std::string> boundary_ids_to_names =
      mesh.getMesh().get_boundary_info().get_sideset_name_map();

  // 9.
  // node_ids_for_boundary_id maps from the boundary ID to a vector of vectors containing
  // the nodes of each element on the boundary that correspond to the face of the boundary.
  const auto node_ids_for_boundary_id = buildBoundaryNodeIDs(
      mesh, unique_side_boundary_ids, element_ids_for_boundary_id, side_ids_for_boundary_id);

  // 10. Create mapping from the boundary ID to a vector containing the block IDs of all elements
  // that lie on the boundary. This is required for in MFEM mesh for multiple-element types.
  auto block_ids_for_boundary_id =
      getBlockIDsForBoundaryID(element_ids_for_block_id, element_ids_for_boundary_id);

  // 11. Build the serial MFEM mesh.
  LibmeshMFEMMesh serial_mesh(mesh.getMesh().n_active_elem(),
                              block_info,
                              unique_block_ids,
                              block_ids_to_names,
                              unique_side_boundary_ids,
                              boundary_ids_to_names,
                              unique_corner_node_ids,
                              element_ids_for_block_id,
                              node_ids_for_element_id,
                              node_ids_for_boundary_id,
                              side_ids_for_boundary_id,
                              block_ids_for_boundary_id,
                              coordinates_for_node_id);

  // Now use the serial mesh to create a ParMesh, partitioned in the same way as the libMesh mesh.
  auto partitioning =
      getMeshPartitioning(mesh.getMesh(), unique_block_ids, element_ids_for_block_id);
  return std::make_shared<mfem::ParMesh>(mesh.comm().get(), serial_mesh, partitioning.data(), 1);
}
}

#endif
