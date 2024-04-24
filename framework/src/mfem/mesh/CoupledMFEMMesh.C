//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledMFEMMesh.h"

registerMooseObject("PlatypusApp", CoupledMFEMMesh);

InputParameters
CoupledMFEMMesh::validParams()
{
  InputParameters params = ExclusiveMFEMMesh::validParams();
  return params;
}

CoupledMFEMMesh::CoupledMFEMMesh(const InputParameters & parameters)
  : ExclusiveMFEMMesh(parameters), _block_info(_dim)
{
}

CoupledMFEMMesh::~CoupledMFEMMesh() {}

void
CoupledMFEMMesh::buildMesh()
{
  // Use method from file mesh to build MOOSE mesh from Exodus file.
  FileMesh::buildMesh();
}

std::unique_ptr<MooseMesh>
CoupledMFEMMesh::safeClone() const
{
  return std::make_unique<CoupledMFEMMesh>(*this);
}

void
CoupledMFEMMesh::buildBoundaryInfo(std::map<int, std::vector<int>> & element_ids_for_boundary_id,
                                   std::map<int, std::vector<int>> & side_ids_for_boundary_id)
{
  buildBndElemList();

  struct BoundaryElementAndSideIDs
  {
    std::vector<int> element_ids; // Element ids for a boundary id.
    std::vector<int> side_ids;    // Side ids for a boundary id.

    BoundaryElementAndSideIDs() : element_ids{}, side_ids{} {}
  };

  std::vector<BoundaryID> unique_boundary_ids;
  std::map<BoundaryID, BoundaryElementAndSideIDs> boundary_ids_map;

  // Iterate over elements on the boundary to build the map that allows us to go
  // from a boundary id to a vector of element id/side ids.
  for (auto boundary_element : _bnd_elems)
  {
    auto boundary_id = boundary_element->_bnd_id;

    bool is_new_boundary_id = (boundary_ids_map.count(boundary_id) == 0);

    if (is_new_boundary_id) // Initialize new struct.
    {
      boundary_ids_map[boundary_id] = BoundaryElementAndSideIDs();
      unique_boundary_ids.push_back(boundary_id);
    }

    auto element_id = boundary_element->_elem->id(); // ID of element on boundary.
    auto side_id = boundary_element->_side;          // ID of side that element is on.

    boundary_ids_map[boundary_id].element_ids.push_back(element_id);
    boundary_ids_map[boundary_id].side_ids.push_back(side_id);
  }

  // Sort.
  std::sort(unique_boundary_ids.begin(), unique_boundary_ids.end());

  // Run through the (key, value) pairs in the boundary_ids_map map.
  for (const auto & key_value_pair : boundary_ids_map)
  {
    auto boundary_id = key_value_pair.first;

    auto element_ids = key_value_pair.second.element_ids;
    auto side_ids = key_value_pair.second.side_ids;

    element_ids_for_boundary_id[boundary_id] = std::move(element_ids);
    side_ids_for_boundary_id[boundary_id] = std::move(side_ids);
  }
}

std::vector<int>
CoupledMFEMMesh::getSideBoundaryIDs() const
{
  const libMesh::BoundaryInfo & boundary_info = getMesh().get_boundary_info();
  const std::set<boundary_id_type> & side_boundary_ids_set = boundary_info.get_side_boundary_ids();

  std::vector<int> side_boundary_ids(side_boundary_ids_set.size());

  int counter = 0;
  for (auto side_boundary_id : side_boundary_ids_set)
  {
    side_boundary_ids[counter++] = side_boundary_id;
  }

  std::sort(side_boundary_ids.begin(), side_boundary_ids.end());

  return side_boundary_ids;
}

bool
CoupledMFEMMesh::isDistributedMesh() const
{
  return (!getMesh().is_replicated() && n_processors() > 1);
}

void
CoupledMFEMMesh::buildCubitBlockInfo(const std::vector<int> & unique_block_ids)
{
  /**
   * Iterate over the block_ids. Note that we only need to extract the first element from
   * each block since only a single element type can be specified per block.
   */
  for (int block_id : unique_block_ids)
  {
    auto element_range = getMesh().active_subdomain_elements_ptr_range(block_id);
    if (element_range.begin() == element_range.end())
    {
      mooseError("Block '", block_id, "' contains no elements.");
    }

    auto first_element_ptr = *element_range.begin();

    blockInfo().addBlockElement(block_id, first_element_ptr->n_nodes());
  }
}

std::vector<int>
CoupledMFEMMesh::getLibmeshBlockIDs() const
{
  auto & libmesh = getMesh();

  // Identify all subdomains (blocks) in the entire mesh (global == true).
  std::set<subdomain_id_type> block_ids_set;
  libmesh.subdomain_ids(block_ids_set, true);

  std::vector<int> unique_block_ids(block_ids_set.size());

  int counter = 0;
  for (auto block_id : block_ids_set)
  {
    unique_block_ids[counter++] = block_id;
  }

  return unique_block_ids;
}

void
CoupledMFEMMesh::buildElementAndNodeIDs(const std::vector<int> & unique_block_ids,
                                        std::map<int, std::vector<int>> & element_ids_for_block_id,
                                        std::map<int, std::vector<int>> & node_ids_for_element_id)
{
  for (int block_id : unique_block_ids)
  {
    auto & element_info = blockElement(block_id);

    std::vector<int> elements_in_block;

    auto active_block_elements_begin = getMesh().active_subdomain_elements_begin(block_id);
    auto active_block_elements_end = getMesh().active_subdomain_elements_end(block_id);

    for (auto element_iterator = active_block_elements_begin;
         element_iterator != active_block_elements_end;
         element_iterator++)
    {
      auto element_ptr = *element_iterator;

      const int element_id = element_ptr->id();

      std::vector<int> element_node_ids(element_info.numNodes());

      elements_in_block.push_back(element_id);

      for (int node_counter = 0; node_counter < element_info.numNodes(); node_counter++)
      {
        element_node_ids[node_counter] = element_ptr->node_id(node_counter);
      }

      node_ids_for_element_id[element_id] = std::move(element_node_ids);
    }

    elements_in_block.shrink_to_fit();

    // Add to map.
    element_ids_for_block_id[block_id] = std::move(elements_in_block);
  }
}

void
CoupledMFEMMesh::buildUniqueCornerNodeIDs(
    std::vector<int> & unique_corner_node_ids,
    const std::vector<int> & unique_block_ids,
    const std::map<int, std::vector<int>> & element_ids_for_block_id,
    const std::map<int, std::vector<int>> & node_ids_for_element_id)
{
  // Iterate through all nodes (on edge of each element) and add their global IDs
  // to the unique_corner_node_ids vector.
  for (int block_id : unique_block_ids)
  {
    auto & block_element = blockElement(block_id);

    auto & element_ids = element_ids_for_block_id.at(block_id);

    for (int element_id : element_ids)
    {
      auto & node_ids = node_ids_for_element_id.at(element_id);

      // Only use the nodes on the edge of the element!
      for (int knode = 0; knode < block_element.numCornerNodes(); knode++)
      {
        unique_corner_node_ids.push_back(node_ids[knode]);
      }
    }
  }

  // Sort unique_vertex_ids in ascending order and remove duplicate node IDs.
  std::sort(unique_corner_node_ids.begin(), unique_corner_node_ids.end());

  auto new_end = std::unique(unique_corner_node_ids.begin(), unique_corner_node_ids.end());

  unique_corner_node_ids.resize(std::distance(unique_corner_node_ids.begin(), new_end));
}

void
CoupledMFEMMesh::buildMFEMMesh()
{
  // 1. If the mesh is distributed and split between more than one processor,
  // we need to call allgather on each processor. This will gather the nodes
  // and elements onto each processor.
  if (isDistributedMesh())
  {
    getMesh().allgather();
  }

  // 2. Get the unique libmesh IDs of each block in the mesh.
  std::vector<int> unique_block_ids = getLibmeshBlockIDs();

  // 3. Retrieve information about the elements used within the mesh.
  buildCubitBlockInfo(unique_block_ids);

  // 4. Build maps:
  // Map from block ID --> vector of element IDs.
  // Map from element ID --> vector of global node IDs.
  std::map<int, std::vector<int>> element_ids_for_block_id;
  std::map<int, std::vector<int>> node_ids_for_element_id;

  buildElementAndNodeIDs(unique_block_ids, element_ids_for_block_id, node_ids_for_element_id);

  // 5. Create vector containing the IDs of all nodes that are on the corners of
  // elements. MFEM only requires the corner nodes.
  std::vector<int> unique_corner_node_ids;

  buildUniqueCornerNodeIDs(
      unique_corner_node_ids, unique_block_ids, element_ids_for_block_id, node_ids_for_element_id);

  // 6. Create a map to hold the x, y, z coordinates for each unique node.
  std::map<int, std::array<double, 3>> coordinates_for_node_id;

  for (auto node_ptr : getMesh().node_ptr_range())
  {
    auto & node = *node_ptr;

    std::array<double, 3> coordinates = {node(0), node(1), node(2)};

    coordinates_for_node_id[node.id()] = std::move(coordinates);
  }

  // 7.
  // element_ids_for_boundary_id stores the ids of each element on each boundary.
  // side_ids_for_boundary_id stores the sides of those elements that are on each boundary.
  std::map<int, std::vector<int>> element_ids_for_boundary_id;
  std::map<int, std::vector<int>> side_ids_for_boundary_id;

  buildBoundaryInfo(element_ids_for_boundary_id, side_ids_for_boundary_id);

  // 8. Get a vector containing all boundary IDs on sides of semi-local elements.
  std::vector<int> unique_side_boundary_ids = getSideBoundaryIDs();

  // 9.
  // node_ids_for_boundary_id maps from the boundary ID to a vector of vectors containing
  // the nodes of each element on the boundary that correspond to the face of the boundary.
  std::map<int, std::vector<std::vector<unsigned int>>> node_ids_for_boundary_id;

  buildBoundaryNodeIDs(unique_side_boundary_ids,
                       element_ids_for_boundary_id,
                       side_ids_for_boundary_id,
                       node_ids_for_boundary_id);

  // 10. Create mapping from the boundary ID to a vector containing the block IDs of all elements
  // that lie on the boundary. This is required for in MFEM mesh for multiple-element types.
  auto block_ids_for_boundary_id =
      getBlockIDsForBoundaryID(element_ids_for_block_id, element_ids_for_boundary_id);

  // 11.
  // Call the correct initializer.
  switch (blockInfo().order())
  {
    case 1:
    {
      _mfem_mesh = std::make_shared<MFEMMesh>(nElem(),
                                              blockInfo(),
                                              unique_block_ids,
                                              unique_side_boundary_ids,
                                              unique_corner_node_ids,
                                              element_ids_for_block_id,
                                              node_ids_for_element_id,
                                              node_ids_for_boundary_id,
                                              side_ids_for_boundary_id,
                                              block_ids_for_boundary_id,
                                              coordinates_for_node_id);
      break;
    }
    case 2:
    {
      _mfem_mesh = std::make_shared<MFEMMesh>(nElem(),
                                              blockInfo(),
                                              unique_block_ids,
                                              unique_side_boundary_ids,
                                              unique_corner_node_ids,
                                              element_ids_for_block_id,
                                              node_ids_for_element_id,
                                              node_ids_for_boundary_id,
                                              side_ids_for_boundary_id,
                                              block_ids_for_boundary_id,
                                              coordinates_for_node_id,
                                              _libmesh_global_node_id_for_mfem_local_node_id,
                                              _mfem_local_node_id_for_libmesh_global_node_id);
      break;
    }
    default:
    {
      mooseError("Unsupported element type of order ", blockInfo().order(), ".");
      break;
    }
  }
}

std::map<int, int>
CoupledMFEMMesh::getBlockIDForElementID(
    const std::map<int, std::vector<int>> & element_ids_for_block_id) const
{
  std::map<int, int> block_id_for_element_id;

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

std::map<int, std::vector<int>>
CoupledMFEMMesh::getBlockIDsForBoundaryID(
    const std::map<int, std::vector<int>> & element_ids_for_block_id,
    const std::map<int, std::vector<int>> & element_ids_for_boundary_id) const
{
  auto block_id_for_element_id = getBlockIDForElementID(element_ids_for_block_id);

  std::map<int, std::vector<int>> block_ids_for_boundary_id;

  for (const auto & key_value : element_ids_for_boundary_id)
  {
    auto boundary_id = key_value.first;
    auto & element_ids = key_value.second;

    std::vector<int> block_ids(element_ids.size());

    int ielement = 0;
    for (const auto & element_id : element_ids)
    {
      block_ids[ielement++] = block_id_for_element_id.at(element_id);
    }

    block_ids_for_boundary_id[boundary_id] = std::move(block_ids);
  }

  return block_ids_for_boundary_id;
}

std::unique_ptr<int[]>
CoupledMFEMMesh::getMeshPartitioning()
{
  // Call allgather because we need all element information on each processor.
  getMesh().allgather();

  const MeshBase & lib_mesh = getMesh();

  const int num_elements = lib_mesh.n_elem();
  if (num_elements < 1)
  {
    return nullptr;
  }

  int * mesh_partitioning = new int[num_elements];

  for (auto element : lib_mesh.element_ptr_range())
  {
    int element_id = element->id();

    mesh_partitioning[element_id] = element->processor_id();
  }

  // Wrap-up in a unique pointer.
  return std::unique_ptr<int[]>(mesh_partitioning);
}

void
CoupledMFEMMesh::buildMFEMParMesh()
{
  auto partitioning = getMeshPartitioning();

  int * partitioning_raw_ptr = partitioning ? partitioning.get() : nullptr;

  _mfem_par_mesh =
      std::make_shared<MFEMParMesh>(MPI_COMM_WORLD, getMFEMMesh(), partitioning_raw_ptr);

  // If we have a higher-order mesh then we need to figure-out the mapping from the libMesh node ID
  // to the MFEM node ID since this will have changed.
  convertSerialDofMappingsToParallel(*_mfem_mesh.get(), *_mfem_par_mesh.get());

  _mfem_mesh.reset(); // Lower reference count of serial mesh since no longer needed.
}

void
CoupledMFEMMesh::convertSerialDofMappingsToParallel(const MFEMMesh & serial_mesh,
                                                    const MFEMParMesh & parallel_mesh)
{
  // No need to change dof mappings if running on a single processor or if a first order element.
  if (n_processors() < 2 || blockInfo().order() == 1)
  {
    return;
  }

  // Get the FE spaces.
  const auto * serial_fespace = serial_mesh.GetNodalFESpace();
  const auto * parallel_fespace = parallel_mesh.GetNodalFESpace();

  mooseAssert(serial_fespace != nullptr && parallel_fespace != nullptr, "Nodal FESpace is NULL!");

  // Important notes:
  // 1. LibMesh: node id is unique even across multiple processors.
  // 2. MFEM: "local dof": belongs to the processor.
  // 3. MFEM: "local true dof": unique nodes on a processor. i.e. if local nodes 1, 2 both
  // correspond to the same coordinates on a processor they will map to a single true dof.
  std::map<int, int> libmesh_global_node_id_for_mfem_local_node_id;
  std::map<int, int> mfem_local_node_id_for_libmesh_global_node_id;

  // Match-up the libMesh elements on the processor with the MFEM elements on the ParMesh.
  int counter = 0;
  for (auto element_ptr : getMesh().local_element_ptr_range())
  {
    const auto global_element_id = element_ptr->id();
    const auto local_element_id = counter++;

    // Get the LOCAL dofs of the element for both the serial and ParMesh for this element.
    mfem::Array<int> serial_local_dofs;
    serial_fespace->GetElementDofs(global_element_id, serial_local_dofs);

    mfem::Array<int> parallel_local_dofs;
    parallel_fespace->GetElementDofs(local_element_id, parallel_local_dofs);

    // Verify that the number of LOCAL dofs match.
    mooseAssert(serial_local_dofs.Size() == parallel_local_dofs.Size(),
                "Serial and parallel dofs sizes do not match for element.");

    const auto num_local_dofs = serial_local_dofs.Size();

    std::map<int, int> serial_dof_for_parallel_dof;
    std::map<int, int> parallel_dof_for_serial_dof;

    for (int i = 0; i < num_local_dofs; i++)
    {
      auto parallel_dof = parallel_local_dofs[i];
      auto serial_dof = serial_local_dofs[i];

      serial_dof_for_parallel_dof[parallel_dof] = serial_dof;
      parallel_dof_for_serial_dof[serial_dof] = parallel_dof;
    }

    // Iterate over serial local dofs and update mappings.
    for (auto dof : serial_local_dofs)
    {
      const auto parallel_local_dof = parallel_dof_for_serial_dof[dof];

      // Pair the libmesh node formerly associated with the serial node with this node.
      auto libmesh_node_id = getLibmeshGlobalNodeId(dof);

      // Update two-way mappings.
      libmesh_global_node_id_for_mfem_local_node_id[parallel_local_dof] = libmesh_node_id;
      mfem_local_node_id_for_libmesh_global_node_id[libmesh_node_id] = parallel_local_dof;
    }
  }

  // Set two-way mappings.
  _libmesh_global_node_id_for_mfem_local_node_id = libmesh_global_node_id_for_mfem_local_node_id;
  _mfem_local_node_id_for_libmesh_global_node_id = mfem_local_node_id_for_libmesh_global_node_id;
}

void
CoupledMFEMMesh::buildBoundaryNodeIDs(
    const std::vector<int> & unique_side_boundary_ids,
    const std::map<int, std::vector<int>> & element_ids_for_boundary_id,
    const std::map<int, std::vector<int>> & side_ids_for_boundary_id,
    std::map<int, std::vector<std::vector<unsigned int>>> & node_ids_for_boundary_id)
{
  node_ids_for_boundary_id.clear();

  // Iterate over all boundary IDs.
  for (int boundary_id : unique_side_boundary_ids)
  {
    // Get element IDs of element on boundary (and their sides that are on boundary).
    auto & boundary_element_ids = element_ids_for_boundary_id.at(boundary_id);
    auto & boundary_element_sides = side_ids_for_boundary_id.at(boundary_id);

    // Create vector to store the node ids of all boundary nodes.
    std::vector<std::vector<unsigned int>> boundary_node_ids(boundary_element_ids.size());

    // Iterate over elements on boundary.
    for (int jelement = 0; jelement < (int)boundary_element_ids.size(); jelement++)
    {
      // Get element ID and the boundary side.
      const int boundary_element_global_id = boundary_element_ids[jelement];
      const int boundary_element_side = boundary_element_sides[jelement];

      Elem * element_ptr = elemPtr(boundary_element_global_id);

      // Get vector of local node IDs on boundary side of element.
      auto nodes_of_element_on_side = element_ptr->nodes_on_side(boundary_element_side);

      // Replace local IDs with global IDs.
      for (int knode = 0; knode < (int)nodes_of_element_on_side.size(); knode++)
      {
        // Get the global node ID of each node.
        const int local_node_id = nodes_of_element_on_side[knode];
        const int global_node_id = element_ptr->node_id(local_node_id);

        nodes_of_element_on_side[knode] = global_node_id;
      }

      // Add to vector.
      boundary_node_ids[jelement] = std::move(nodes_of_element_on_side);
    }

    // Add to the map.
    node_ids_for_boundary_id[boundary_id] = std::move(boundary_node_ids);
  }
}
