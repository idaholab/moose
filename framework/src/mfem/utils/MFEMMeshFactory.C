#include "MFEMMeshFactory.h"
#include "libmesh/elem.h"
#include "libmesh/enum_io_package.h"
#include "libmesh/equation_systems.h"
#include "libmesh/face_quad4.h"
#include "libmesh/ignore_warnings.h"
#include "libmesh/libmesh_config.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_input.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"
#include "libmesh/vtk_io.h"
#include "CubitElementInfo.h"
#include "LibmeshMFEMMesh.h"
#include "MooseMesh.h"
#include "MFEMMesh.h"
#include <memory>
#include <map>
#include <tuple>
#include <vector>

std::shared_ptr<mfem::ParMesh>
buildMFEMMesh(MooseMesh & mesh)
{
  // If working with an MFEMMesh object then the underlying
  // `mfem::ParMesh` is already available.
  if (mesh.type() == "MFEMMesh")
  {
    return static_cast<MFEMMesh &>(mesh).getMFEMParMeshPtr();
  }
  // Otherwise we must construct it ourselves. Start by building a serial mesh.

  // 1. If the mesh is distributed and split between more than one processor,
  // we need to call allgather on each processor. This will gather the nodes
  // and elements onto each processor.
  if (mesh.isDistributedMesh())
  {
    mesh.getMesh().allgather();
  }

  // 2. Get the unique libmesh IDs of each block in the mesh.
  std::vector<int> unique_block_ids = getLibmeshBlockIDs(mesh.getMesh());

  // 3. Retrieve information about the elements used within the mesh.
  CubitBlockInfo block_info = buildCubitBlockInfo(mesh.getMesh(), unique_block_ids);

  // 4. Build maps:
  // Map from block ID --> vector of element IDs.
  // Map from element ID --> vector of global node IDs.
  auto [element_ids_for_block_id, node_ids_for_element_id] =
      buildElementAndNodeIDs(mesh.getMesh(), block_info, unique_block_ids);

  // 5. Create vector containing the IDs of all nodes that are on the corners of
  // elements. MFEM only requires the corner nodes.
  std::vector<int> unique_corner_node_ids = buildUniqueCornerNodeIDs(
      block_info, unique_block_ids, element_ids_for_block_id, node_ids_for_element_id);

  // 6. Create a map to hold the x, y, z coordinates for each unique node.
  std::map<int, std::array<double, 3>> coordinates_for_node_id;

  for (auto node_ptr : mesh.getMesh().node_ptr_range())
  {
    auto & node = *node_ptr;

    std::array<double, 3> coordinates = {node(0), node(1), node(2)};

    coordinates_for_node_id[node.id()] = std::move(coordinates);
  }

  // 7.
  // element_ids_for_boundary_id stores the ids of each element on each boundary.
  // side_ids_for_boundary_id stores the sides of those elements that are on each boundary.
  auto [element_ids_for_boundary_id, side_ids_for_boundary_id] = buildBoundaryInfo(mesh);

  // 8. Get a vector containing all boundary IDs on sides of semi-local elements.
  std::vector<int> unique_side_boundary_ids = getSideBoundaryIDs(mesh.getMesh());

  // 9.
  // node_ids_for_boundary_id maps from the boundary ID to a vector of vectors containing
  // the nodes of each element on the boundary that correspond to the face of the boundary.
  std::map<int, std::vector<std::vector<unsigned int>>> node_ids_for_boundary_id =
      buildBoundaryNodeIDs(
          mesh, unique_side_boundary_ids, element_ids_for_boundary_id, side_ids_for_boundary_id);

  // 10. Create mapping from the boundary ID to a vector containing the block IDs of all elements
  // that lie on the boundary. This is required for in MFEM mesh for multiple-element types.
  auto block_ids_for_boundary_id =
      getBlockIDsForBoundaryID(element_ids_for_block_id, element_ids_for_boundary_id);

  // FIXME: Alex's libmeshtomfemmesh branch
  // (https://github.com/idaholab/moose/compare/next...alexanderianblair:platypus-moose:alexanderianblair/libmeshtomfemmesh)
  // moves some things out of the LibmeshMFEMMesh constructor and into
  // this function. I guess a benefit to doing that is it makes it
  // clear what can be achieved with public methods and what still
  // requires access to protected members.
  //
  // If I do that then I should probably move the logic into separate
  // functions, to keep this one from getting too big. Arguably I
  // should do that already. (E.g., go back to separate functions for
  // mfem::ParMesh and mfem::Mesh.)

  // 11.
  // Call the correct initializer.
  std::map<int, int> _libmesh_global_node_id_for_mfem_local_node_id,
      _mfem_local_node_id_for_libmesh_global_node_id;

  std::shared_ptr<LibmeshMFEMMesh> _mfem_mesh;
  switch (block_info.order())
  {
    case 1:
    {
      _mfem_mesh = std::make_shared<LibmeshMFEMMesh>(mesh.nElem(),
                                                     block_info,
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
    case 3:
    {
      _mfem_mesh =
          std::make_shared<LibmeshMFEMMesh>(mesh.nElem(),
                                            block_info,
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
      mooseError("Unsupported element type of order ", block_info.order(), ".");
      break;
    }
  }

  // Now use the serial mesh to create a ParMesh

  auto partitioning = getMeshPartitioning(mesh.getMesh());
  int * partitioning_raw_ptr = partitioning ? partitioning.get() : nullptr;

  auto _mfem_par_mesh =
      std::make_shared<mfem::ParMesh>(MPI_COMM_WORLD, *_mfem_mesh, partitioning_raw_ptr, 1);

  // If we have a higher-order mesh then we need to figure-out the mapping from the libMesh node ID
  // to the MFEM node ID since this will have changed.
  // No need to change dof mappings if running on a single processor or if a first order element.
  if (mesh.n_processors() > 1 && block_info.order() > 1)
  {
    convertSerialDofMappingsToParallel(mesh.getMesh(),
                                       *_mfem_mesh.get(),
                                       *_mfem_par_mesh.get(),
                                       _libmesh_global_node_id_for_mfem_local_node_id,
                                       _mfem_local_node_id_for_libmesh_global_node_id);
  }

  // FIXME: What are _libmesh_global_node_id_for_mfem_local_node_id
  // and _mfem_local_node_id_for_libmesh_global_node_ID actually used
  // for? They don't ever seem to be accessed. Perhaps they will need
  // to be returned and used once we do transfers between libmesh and
  // mfem objects?

  //_mfem_mesh.reset(); // Lower reference count of serial mesh since no longer needed.
  return _mfem_par_mesh;
}

std::tuple<IDMap, IDMap>
buildBoundaryInfo(MooseMesh & mesh)
{
  IDMap element_ids_for_boundary_id;
  IDMap side_ids_for_boundary_id;
  mesh.buildBndElemList();

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
  for (MooseMesh::bnd_elem_iterator boundary_element = mesh.bndElemsBegin();
       boundary_element != mesh.bndElemsEnd();
       ++boundary_element)
  {
    auto boundary_id = (*boundary_element)->_bnd_id;

    bool is_new_boundary_id = (boundary_ids_map.count(boundary_id) == 0);

    if (is_new_boundary_id) // Initialize new struct.
    {
      boundary_ids_map[boundary_id] = BoundaryElementAndSideIDs();
      unique_boundary_ids.push_back(boundary_id);
    }

    auto element_id = (*boundary_element)->_elem->id(); // ID of element on boundary.
    auto side_id = (*boundary_element)->_side;          // ID of side that element is on.

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

  return {element_ids_for_boundary_id, side_ids_for_boundary_id};
}

std::map<int, std::vector<std::vector<unsigned int>>>
buildBoundaryNodeIDs(const MooseMesh & mesh,
                     const std::vector<int> & unique_side_bound_ids,
                     const IDMap & element_ids_for_bound,
                     const IDMap & side_ids_for_bound)
{
  std::map<int, std::vector<std::vector<unsigned int>>> node_ids_for_bound_id;

  // Iterate over all bound IDs.
  for (int bound_id : unique_side_bound_ids)
  {
    // Get element IDs of element on bound (and their sides that are on bound).
    auto & bound_element_ids = element_ids_for_bound.at(bound_id);
    auto & bound_element_sides = side_ids_for_bound.at(bound_id);

    // Create vector to store the node ids of all bound nodes.
    std::vector<std::vector<unsigned int>> bound_node_ids(bound_element_ids.size());

    // Iterate over elements on bound.
    for (int jelement = 0; jelement < (int)bound_element_ids.size(); jelement++)
    {
      // Get element ID and the bound side.
      const int bound_element_global_id = bound_element_ids[jelement];
      const int bound_element_side = bound_element_sides[jelement];

      const Elem * element_ptr = mesh.elemPtr(bound_element_global_id);

      // Get vector of local node IDs on bound side of element.
      auto nodes_of_element_on_side = element_ptr->nodes_on_side(bound_element_side);

      // Replace local IDs with global IDs.
      for (int knode = 0; knode < (int)nodes_of_element_on_side.size(); knode++)
      {
        // Get the global node ID of each node.
        const int local_node_id = nodes_of_element_on_side[knode];
        const int global_node_id = element_ptr->node_id(local_node_id);

        nodes_of_element_on_side[knode] = global_node_id;
      }

      // Add to vector.
      bound_node_ids[jelement] = std::move(nodes_of_element_on_side);
    }

    // Add to the map.
    node_ids_for_bound_id[bound_id] = std::move(bound_node_ids);
  }

  return node_ids_for_bound_id;
}

std::tuple<IDMap, IDMap>
buildElementAndNodeIDs(MeshBase & libmesh,
                       const CubitBlockInfo block_info,
                       const std::vector<int> & unique_block_ids)
{
  IDMap element_ids_for_block_id;
  IDMap node_ids_for_element_id;

  for (int block_id : unique_block_ids)
  {
    auto & element_info = block_info.blockElement(block_id);

    std::vector<int> elements_in_block;

    // MFEM block IDs are 1-indexed, while libmesh ones are 0-indexed, so offset by 1.
    auto active_block_elements_begin = libmesh.active_subdomain_elements_begin(block_id - 1);
    auto active_block_elements_end = libmesh.active_subdomain_elements_end(block_id - 1);

    for (auto element_iterator = active_block_elements_begin;
         element_iterator != active_block_elements_end;
         element_iterator++)
    {
      auto element_ptr = *element_iterator;

      const int element_id = element_ptr->id();

      std::vector<int> element_node_ids(element_info.num_nodes);

      elements_in_block.push_back(element_id);

      for (int node_counter = 0; node_counter < element_info.num_nodes; node_counter++)
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

std::vector<int>
buildUniqueCornerNodeIDs(const CubitBlockInfo & block_info,
                         const std::vector<int> & unique_block_ids,
                         const IDMap & element_ids_for_block_id,
                         const IDMap & node_ids_for_element_id)
{
  std::vector<int> unique_corner_node_ids;

  // Iterate through all nodes (on edge of each element) and add their global IDs
  // to the unique_corner_node_ids vector.
  for (int block_id : unique_block_ids)
  {
    auto & block_element = block_info.blockElement(block_id);
    auto & element_ids = element_ids_for_block_id.at(block_id);
    for (int element_id : element_ids)
    {
      auto & node_ids = node_ids_for_element_id.at(element_id);

      // Only use the nodes on the edge of the element!
      for (int knode = 0; knode < block_element.num_corner_nodes; knode++)
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

void
convertSerialDofMappingsToParallel(
    const MeshBase & libmesh,
    const mfem::Mesh & serial_mesh,
    const mfem::ParMesh & parallel_mesh,
    std::map<int, int> & libmesh_global_node_id_for_mfem_local_node_id,
    std::map<int, int> & mfem_local_node_id_for_libmesh_global_node_id)
{
  // Get the FE spaces.
  const auto * serial_fespace = serial_mesh.GetNodalFESpace();
  const auto * parallel_fespace = parallel_mesh.GetNodalFESpace();

  mooseAssert(serial_fespace != nullptr && parallel_fespace != nullptr, "Nodal FESpace is NULL!");

  // Important notes:
  // 1. LibMesh: node id is unique even across multiple processors.
  // 2. MFEM: "local dof": belongs to the processor.
  // 3. MFEM: "local true dof": unique nodes on a processor. i.e. if local nodes 1, 2 both
  // correspond to the same coordinates on a processor they will map to a single true dof.
  std::map<int, int> _libmesh_global_node_id_for_mfem_local_node_id;
  std::map<int, int> _mfem_local_node_id_for_libmesh_global_node_id;

  // Match-up the libMesh elements on the processor with the MFEM elements on the ParMesh.
  int counter = 0;
  for (auto element_ptr : libmesh.local_element_ptr_range())
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
      const int libmesh_node_id = libmesh_global_node_id_for_mfem_local_node_id.at(dof);

      // Update two-way mappings.
      _libmesh_global_node_id_for_mfem_local_node_id[parallel_local_dof] = libmesh_node_id;
      _mfem_local_node_id_for_libmesh_global_node_id[libmesh_node_id] = parallel_local_dof;
    }
  }

  libmesh_global_node_id_for_mfem_local_node_id = _libmesh_global_node_id_for_mfem_local_node_id;
  mfem_local_node_id_for_libmesh_global_node_id = _mfem_local_node_id_for_libmesh_global_node_id;
}

CubitBlockInfo
buildCubitBlockInfo(MeshBase & libmesh, const std::vector<int> & unique_block_ids)
{
  CubitBlockInfo block_info(libmesh.mesh_dimension());
  /**
   * Iterate over the block_ids. Note that we only need to extract the first element from
   * each block since only a single element type can be specified per block.
   */
  for (int block_id : unique_block_ids)
  {
    // MFEM block IDs are 1-indexed, while libmesh ones are 0-indexed, so offset by 1.
    auto element_range = libmesh.active_subdomain_elements_ptr_range(block_id - 1);
    if (element_range.begin() == element_range.end())
    {
      mooseError("Block '", block_id - 1, "' contains no elements.");
    }

    auto first_element_ptr = *element_range.begin();

    block_info.addBlockElement(
        block_id, first_element_ptr->type(), first_element_ptr->mapping_type());
  }
  return block_info;
}

std::vector<int>
getLibmeshBlockIDs(const MeshBase & libmesh)
{
  // Identify all subdomains (blocks) in the entire mesh (global == true).
  std::set<subdomain_id_type> block_ids_set;
  libmesh.subdomain_ids(block_ids_set, true);

  std::vector<int> unique_block_ids(block_ids_set.size());

  int counter = 0;
  for (auto block_id : block_ids_set)
  {
    // MFEM block IDs are 1-indexed, while libmesh ones are 0-indexed, so offset by 1.
    unique_block_ids[counter++] = block_id + 1;
  }

  return unique_block_ids;
}

std::vector<int>
getSideBoundaryIDs(const MeshBase & libmesh)
{
  const libMesh::BoundaryInfo & boundary_info = libmesh.get_boundary_info();
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

std::map<int, int>
getBlockIDForElementID(const std::map<int, std::vector<int>> & element_ids_for_block_id)
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

IDMap
getBlockIDsForBoundaryID(const std::map<int, std::vector<int>> & element_ids_for_block_id,
                         const std::map<int, std::vector<int>> & element_ids_for_boundary_id)
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
getMeshPartitioning(MeshBase & libmesh)
{
  // Call allgather because we need all element information on each processor.
  libmesh.allgather();

  const int num_elements = libmesh.n_elem();
  if (num_elements < 1)
  {
    return nullptr;
  }

  int * mesh_partitioning = new int[num_elements];

  for (auto element : libmesh.element_ptr_range())
  {
    int element_id = element->id();

    mesh_partitioning[element_id] = element->processor_id();
  }

  // Wrap-up in a unique pointer.
  return std::unique_ptr<int[]>(mesh_partitioning);
}
