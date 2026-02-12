//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlockGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/partitioner.h"
#include "timpi/parallel_sync.h"

registerMooseObject("MooseApp", BreakMeshByBlockGenerator);

InputParameters
BreakMeshByBlockGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Break the mesh at interfaces between blocks. New nodes will be generated so elements on "
      "each side of the break are no longer connected.");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  params.addParam<std::string>(
      "interface_name",
      "interface",
      "the name of the new interface. Cannot be used with `split_interface=true`");
  params.addParam<bool>("split_interface",
                        true,
                        "If true, it creates a "
                        "different interface for each block pair.");
  params.addParam<std::vector<SubdomainName>>(
      "surrounding_blocks",
      "The list of subdomain names surrounding which interfaces will be generated.");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "block_pairs", "The list of subdomain pairs between which interfaces will be generated.");
  params.addParam<bool>("add_transition_interface",
                        false,
                        "If true and block is not empty, a special boundary named "
                        "interface_transition is generate between listed blocks and other blocks.");
  params.addParam<bool>(
      "split_transition_interface", false, "Whether to split the transition interface by blocks.");
  params.addParam<bool>("add_interface_on_two_sides",
                        true,
                        "Whether to add an additional interface boundary at the other side.");
  params.addParam<BoundaryName>(
      "interface_transition_name",
      "interface_transition",
      "the name of the interface transition boundary created when blocks are provided");

  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 1; });

  return params;
}

BreakMeshByBlockGenerator::BreakMeshByBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _interface_name(getParam<std::string>("interface_name")),
    _split_interface(getParam<bool>("split_interface")),
    _block_pairs_restricted(parameters.isParamSetByUser("block_pairs")),
    _surrounding_blocks_restricted(parameters.isParamSetByUser("surrounding_blocks")),
    _add_transition_interface(getParam<bool>("add_transition_interface")),
    _split_transition_interface(getParam<bool>("split_transition_interface")),
    _interface_transition_name(getParam<BoundaryName>("interface_transition_name")),
    _add_interface_on_two_sides(getParam<bool>("add_interface_on_two_sides"))
{
  if (getParam<bool>("split_interface") && _pars.isParamSetByUser("interface_name"))
    paramError("interface_name",
               "if split_interface == true, the new interface_name cannot be specified by the "
               "user. It will be automatically assigned");
  if (_block_pairs_restricted && _surrounding_blocks_restricted)
    paramError("block_pairs_restricted",
               "BreakMeshByBlockGenerator: 'surrounding_blocks' and 'block_pairs' can not be used "
               "at the same time.");

  if (!_add_transition_interface && _split_transition_interface)
    paramError("split_transition_interface",
               "BreakMeshByBlockGenerator cannot split the transition interface because "
               "add_transition_interface is false");

  if (_add_transition_interface && _block_pairs_restricted)
    paramError("add_transition_interface",
               "BreakMeshByBlockGenerator cannot split the transition interface when 'block_pairs' "
               "are specified.");
}

BoundaryName
BreakMeshByBlockGenerator::generateBoundaryName(const MeshBase & mesh,
                                                subdomain_id_type primaryBlockID,
                                                subdomain_id_type secondaryBlockID)
{
  std::string primary_block_name = mesh.subdomain_name(primaryBlockID);
  std::string secondary_block_name = mesh.subdomain_name(secondaryBlockID);
  if (primary_block_name.empty())
    primary_block_name = "Block" + std::to_string(primaryBlockID);
  if (secondary_block_name.empty())
    secondary_block_name = "Block" + std::to_string(secondaryBlockID);

  return primary_block_name + "_" + secondary_block_name;
}

void
BreakMeshByBlockGenerator::mapBoundaryIdAndBoundaryName(boundary_id_type boundaryID,
                                                        const std::string & boundaryName)
{
  _bName_bID_set.insert(std::pair<std::string, int>(boundaryName, boundaryID));
}

void
BreakMeshByBlockGenerator::findBoundaryName(const MeshBase & mesh,
                                            subdomain_id_type primaryBlockID,
                                            subdomain_id_type secondaryBlockID,
                                            BoundaryName & boundaryName,
                                            boundary_id_type boundaryID,
                                            BoundaryInfo & boundary_info)
{
  boundaryName = generateBoundaryName(mesh, primaryBlockID, secondaryBlockID);

  // check if the boundary name already exist
  bool checkBoundaryAlreadyExist = false;
  for (auto b : _bName_bID_set)
  {
    if (b.first.compare(boundaryName) == 0)
    {
      mooseAssert(boundaryID == b.second, "Boundary with two inconsistent ids");
      checkBoundaryAlreadyExist = true;
    }
  }

  if (checkBoundaryAlreadyExist)
    return;
  else
  {
    mapBoundaryIdAndBoundaryName(boundaryID, boundaryName);

    boundary_info.sideset_name(boundaryID) = boundaryName;

    return;
  }
}

std::unique_ptr<MeshBase>
BreakMeshByBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Max node id is used later to generate new unique node IDs
  auto max_node_id = mesh->max_node_id();
  // If the mesh is prepared, max_node_id is already the same on all processors so do not need to
  // communicate.
  if (!mesh->is_replicated() && !mesh->is_prepared())
    mesh->comm().max(max_node_id);
#if LIBMESH_ENABLE_UNIQUE_ID
  const auto max_unique_id = mesh->parallel_max_unique_id();
#endif

  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Handle block restrictions
  if (_block_pairs_restricted)
  {
    for (const auto & block_name_pair :
         getParam<std::vector<std::vector<SubdomainName>>>("block_pairs"))
    {
      if (block_name_pair.size() != 2)
        paramError("block_pairs",
                   "Each row of 'block_pairs' must have a size of two (block names).");

      // check that the blocks exist in the mesh
      for (const auto & name : block_name_pair)
        if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
          paramError("block_pairs", "The block '", name, "' was not found in the mesh");

      const auto block_pair = MooseMeshUtils::getSubdomainIDs(*mesh, block_name_pair);
      std::pair<SubdomainID, SubdomainID> pair = std::make_pair(
          std::min(block_pair[0], block_pair[1]), std::max(block_pair[0], block_pair[1]));

      _block_pairs.insert(pair);
      // Insert all SubdomainIDs from block_pair into _block_set (duplicates automatically
      // ignored)
      std::copy(block_pair.begin(), block_pair.end(), std::inserter(_block_set, _block_set.end()));
    }
  }
  else if (_surrounding_blocks_restricted)
  {
    // check that the blocks exist in the mesh
    for (const auto & name : getParam<std::vector<SubdomainName>>("surrounding_blocks"))
      if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
        paramError("surrounding_blocks", "The block '", name, "' was not found in the mesh");

    const auto surrounding_block_ids = MooseMeshUtils::getSubdomainIDs(
        *mesh, getParam<std::vector<SubdomainName>>("surrounding_blocks"));
    std::copy(surrounding_block_ids.begin(),
              surrounding_block_ids.end(),
              std::inserter(_block_set, _block_set.end()));
  }

  // check that a boundary named _interface_transition_name does not already exist in the mesh
  if ((_block_pairs_restricted || _surrounding_blocks_restricted) && _add_transition_interface &&
      boundary_info.get_id_by_name(_interface_transition_name) != Moose::INVALID_BOUNDARY_ID)
    paramError("interface_transition_name",
               "BreakMeshByBlockGenerator the specified  interface transition boundary name "
               "already exits");

  // initialize the node to element map
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_elem_map;
  for (const auto & elem : mesh->active_element_ptr_range())
    for (const auto n : make_range(elem->n_nodes()))
      node_to_elem_map[elem->node_id(n)].push_back(elem->id());

  // Sync connected block info across all ranks
  // a map from a node id to the set of connected block ids
  const auto nodeid_to_connected_blocks = syncConnectedBlocks(node_to_elem_map, *mesh);

  // Main loop to duplicate nodes
  for (const auto & [current_node_id, connected_elems] : node_to_elem_map)
  {
    const auto * current_node = mesh->node_ptr(current_node_id);
    if (!current_node)
      continue;

    // If the node is not connected to any blocks, skip it
    auto it = nodeid_to_connected_blocks.find(current_node_id);
    if (it == nodeid_to_connected_blocks.end())
      continue;

    // Get the set of blocks connected to the current node.
    // Make a local copy because we'll modify it (e.g., erase Elem::invalid_subdomain_id).
    // Preserve the original map's information (including invalid_subdomain_id) for global
    // synchronization/logic, but operate on the copy for duplication decisions.
    auto connected_blocks = it->second;

    // Remove invalid subdomain if block pairs restriction is active
    if (_block_pairs_restricted)
      connected_blocks.erase(Elem::invalid_subdomain_id);

    const unsigned int node_multiplicity = connected_blocks.size();

    // check if current_node need to be duplicated
    if (node_multiplicity > 1)
    {
      // find reference_subdomain_id (e.g. the subdomain with lower id or the
      // Elem::invalid_subdomain_id)
      auto subdomain_it = connected_blocks.begin();
      subdomain_id_type reference_subdomain_id =
          !_block_pairs_restricted &&
                  connected_blocks.find(Elem::invalid_subdomain_id) != connected_blocks.end()
              ? Elem::invalid_subdomain_id
              : *subdomain_it;

      // For the block_pairs option, the node that is only shared by specific block pairs will be
      // newly created.
      bool should_create_new_node = true;
      if (_block_pairs_restricted)
      {
        // Default to false; only set to true if this is exactly the pair boundary we want
        should_create_new_node = false;

        // Directly use the global info synchronized earlier by syncConnectedBlocks
        // Invalid subdomain is included for correct logic
        const auto & sets_blocks_for_this_node = it->second;

        // Check if this node is exactly at the boundary between two blocks
        // If it is a junction between more than two blocks, we do not split it
        // For the block_pairs option, only nodes shared by the specified block pairs are newly
        // created.
        if (sets_blocks_for_this_node.size() == 2)
        {
          // Get the two block IDs from the set
          auto set_blocks_for_this_node_it = sets_blocks_for_this_node.begin();
          subdomain_id_type block1 = *set_blocks_for_this_node_it;
          subdomain_id_type block2 = *std::next(set_blocks_for_this_node_it);

          // Check if this block pair is one of the user-specified pairs to split
          if (findBlockPairs(block1, block2))
            should_create_new_node = true;
        }
      }

      // multiplicity counter to keep track of how many nodes we added
      unsigned int multiplicity_counter = node_multiplicity;
      for (auto elem_id : connected_elems)
      {
        // all the duplicate nodes are added and assigned
        if (multiplicity_counter == 0)
          break;

        Elem * current_elem = mesh->elem_ptr(elem_id);
        subdomain_id_type block_id = blockRestrictedElementSubdomainID(current_elem);

        // Work on duplicating the node only from the element that is not on the reference subdomain
        if ((block_id != reference_subdomain_id) ||
            (_block_pairs_restricted && findBlockPairs(reference_subdomain_id, block_id)))
        {
          // assign the newly added node to current_elem
          Node * new_node = nullptr;

          std::vector<boundary_id_type> node_boundary_ids;

          for (const auto local_node_id : make_range(current_elem->n_nodes()))
            if (current_elem->node_id(local_node_id) ==
                current_node->id()) // if current node == node on element
            {
              if (should_create_new_node)
              {
                // The maximum number of duplications per subdomain will not exceed the original
                // number of nodes. Since max_node_id is always greater than the original node
                // count, it is safe to use max_node_id as a stride to generate new unique node ID
                // values.
                // max_node_id > original node count > number of duplications per subdomain
                new_node = Node::build(*current_node,
                                       mesh->is_replicated()
                                           ? mesh->n_nodes()
                                           : (current_elem->subdomain_id() + 1) * max_node_id +
                                                 current_node->id())
                               .release();
#if LIBMESH_ENABLE_UNIQUE_ID
                new_node->set_unique_id((current_elem->subdomain_id() + 1) * max_unique_id +
                                        current_node->unique_id());
#endif
                // We're duplicating nodes so that each subdomain elem has its own copy, so it
                // seems natural to assign this new node the same proc id as corresponding
                // subdomain elem
                new_node->processor_id() = current_elem->processor_id();
                mesh->add_node(new_node);
                current_elem->set_node(local_node_id, new_node);
                // Add boundary info to the new node
                boundary_info.boundary_ids(current_node, node_boundary_ids);
                boundary_info.add_node(new_node, node_boundary_ids);
              }
              multiplicity_counter--; // node created, update multiplicity counter
              break; // once the proper node has been fixed in one element we can break the
                     // loop
            }

          if (should_create_new_node)
          {
            for (auto connected_elem_id : connected_elems)
            {
              Elem * connected_elem = mesh->elem_ptr(connected_elem_id);

              // Assign the newly added node to other connected elements with the same
              // block_id
              if (connected_elem->subdomain_id() == current_elem->subdomain_id() &&
                  connected_elem != current_elem)
              {
                for (const auto local_node_id : make_range(connected_elem->n_nodes()))
                  if (connected_elem->node_id(local_node_id) ==
                      current_node->id()) // if current node == node on element
                  {
                    connected_elem->set_node(local_node_id, new_node);
                    break;
                  }
              }
            }
          }
        }
      }

      // create blocks pair and assign element side to new interface boundary map
      for (auto elem_id : connected_elems)
      {
        for (auto connected_elem_id : connected_elems)
        {
          Elem * current_elem = mesh->elem_ptr(elem_id);
          Elem * connected_elem = mesh->elem_ptr(connected_elem_id);

          // Early out if both elements are the same
          if (current_elem == connected_elem)
            continue;

          subdomain_id_type curr_elem_subid = blockRestrictedElementSubdomainID(current_elem);
          subdomain_id_type connected_elem_subid =
              blockRestrictedElementSubdomainID(connected_elem);

          if (curr_elem_subid < connected_elem_subid)
          {
            if (current_elem->has_neighbor(connected_elem))
            {
              unsigned int side = current_elem->which_neighbor_am_i(connected_elem);
              unsigned int connected_elem_side = connected_elem->which_neighbor_am_i(current_elem);

              // in this case we need to play a game to reorder the sides
              // Ensure consistent ordering: (min, max) for block (subdomain) IDs
              bool need_to_switch = false;
              if (_block_pairs_restricted || _surrounding_blocks_restricted)
              {
                connected_elem_subid = connected_elem->subdomain_id();
                if (curr_elem_subid > connected_elem_subid) // we need to switch the ids
                {
                  connected_elem_subid = current_elem->subdomain_id();
                  curr_elem_subid = connected_elem->subdomain_id();

                  side = connected_elem->which_neighbor_am_i(current_elem);

                  connected_elem_side = current_elem->which_neighbor_am_i(connected_elem);
                  need_to_switch = true;
                }
              }

              std::pair<subdomain_id_type, subdomain_id_type> blocks_pair =
                  std::make_pair(curr_elem_subid, connected_elem_subid);

              std::pair<subdomain_id_type, subdomain_id_type> blocks_pair2 =
                  std::make_pair(connected_elem_subid, curr_elem_subid);

              auto add_boundary_sides =
                  [&](const std::pair<subdomain_id_type, subdomain_id_type> & blocks_pair,
                      const std::pair<subdomain_id_type, subdomain_id_type> & blocks_pair2,
                      Elem * current_elem,
                      Elem * connected_elem,
                      unsigned int side,
                      unsigned int connected_elem_side,
                      bool need_to_switch)
              {
                _neighboring_block_list.insert(blocks_pair);
                _subid_pairs_to_sides[blocks_pair].emplace(
                    !need_to_switch ? current_elem : connected_elem, side);
                if (_add_interface_on_two_sides)
                {
                  _neighboring_block_list.insert(blocks_pair2);
                  _subid_pairs_to_sides[blocks_pair2].emplace(
                      !need_to_switch ? connected_elem : current_elem, connected_elem_side);
                }
              };

              if (_block_pairs_restricted)
              {
                if (findBlockPairs(blockRestrictedElementSubdomainID(current_elem),
                                   blockRestrictedElementSubdomainID(connected_elem)))
                  add_boundary_sides(blocks_pair,
                                     blocks_pair2,
                                     current_elem,
                                     connected_elem,
                                     side,
                                     connected_elem_side,
                                     need_to_switch);
              }
              else
                add_boundary_sides(blocks_pair,
                                   blocks_pair2,
                                   current_elem,
                                   connected_elem,
                                   side,
                                   connected_elem_side,
                                   need_to_switch);
            }
          }
        }
      }

    } // end multiplicity check
  } // end loop to duplicate nodes

  addInterface(*mesh);

  // We want to reset node processor ids, which may need to change
  // because of elements being disconnected, but we may have
  // completely disconnected some ghosted nodes on some processors
  // from all ghosted elements that should own them, in which case
  // libMesh on those processors may not be able to tell who to query
  // for an authoritative node processor id.  Let's just delete
  // orphaned nodes so we don't end up with invalid_processor_id on
  // some processors (confusing consistency checks).
  if (!mesh->is_serial())
    mesh->remove_orphaned_nodes();
  Partitioner::set_node_processor_ids(*mesh);

  mesh->set_isnt_prepared();

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
BreakMeshByBlockGenerator::addInterface(MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  boundary_id_type boundary_id;
  boundary_id_type boundary_id_interface = Moose::INVALID_BOUNDARY_ID;
  boundary_id_type boundary_id_interface_transition = Moose::INVALID_BOUNDARY_ID;

  BoundaryName boundary_name;

  const std::set<boundary_id_type> & ids = boundary_info.get_boundary_ids();
  boundary_id_type new_boundaryID = ids.empty() ? 0 : *ids.rbegin() + 1;

  // Make sure the new boundary ID is the same on every processor
  mesh.comm().set_union(_neighboring_block_list);
  mesh.comm().max(new_boundaryID);
  mooseAssert(new_boundaryID >= 0, "Invalid new boundary ID computed.");

  // loop over boundary sides
  // All ranks will process the pairs in exactly the same order.
  std::vector<std::pair<subdomain_id_type, subdomain_id_type>> sorted_pairs(
      _neighboring_block_list.begin(), _neighboring_block_list.end());

  for (auto & boundary_side : sorted_pairs)
  {
    if (!(_block_pairs_restricted || _surrounding_blocks_restricted) ||
        ((_block_pairs_restricted || _surrounding_blocks_restricted) && !_add_transition_interface))
    {
      // find the appropriate boundary name and id
      // given primary and secondary block ID
      if (_split_interface)
      {
        boundary_id = new_boundaryID;
        findBoundaryName(mesh,
                         boundary_side.first,
                         boundary_side.second,
                         boundary_name,
                         boundary_id,
                         boundary_info);
      }
      else
      {
        // When _split_interface == false, all pairs share the same boundary_id_interface
        // (This would cause self-pairing)
        boundary_name = _interface_name;
        // assign a unique boundary ID for the interface boundary
        boundary_id_interface = boundary_id_interface == Moose::INVALID_BOUNDARY_ID
                                    ? new_boundaryID
                                    : boundary_id_interface;
        boundary_id = boundary_id_interface;
        boundary_info.sideset_name(boundary_id_interface) = boundary_name;
      }
    }
    else // block resticted with transition boundary
    {

      const bool interior_boundary = _block_set.find(boundary_side.first) != _block_set.end() &&
                                     _block_set.find(boundary_side.second) != _block_set.end();

      if ((_split_interface && interior_boundary) ||
          (_split_transition_interface && !interior_boundary))
      {
        boundary_id = new_boundaryID;
        findBoundaryName(mesh,
                         boundary_side.first,
                         boundary_side.second,
                         boundary_name,
                         boundary_id,
                         boundary_info);
      }
      else if (interior_boundary)
      {
        boundary_name = _interface_name;
        // assign a unique boundary ID for the interface boundary
        boundary_id_interface = boundary_id_interface == Moose::INVALID_BOUNDARY_ID
                                    ? new_boundaryID
                                    : boundary_id_interface;

        boundary_id = boundary_id_interface;
        boundary_info.sideset_name(boundary_id_interface) = boundary_name;
      }
      else
      {
        // assign a unique boundary ID for the interface transition boundary
        boundary_id_interface_transition =
            boundary_id_interface_transition == Moose::INVALID_BOUNDARY_ID
                ? new_boundaryID
                : boundary_id_interface_transition;
        boundary_id = boundary_id_interface_transition;
        boundary_info.sideset_name(boundary_id) = _interface_transition_name;
      }
    }

    // Map the block pair (subdomain_id_A, subdomain_id_B) to the generated boundary_id
    _subid_pairs_to_boundary_id[boundary_side] = boundary_id;

    // loop over all the side belonging to each pair and add it to the proper interface
    auto boundary_side_map = _subid_pairs_to_sides.find(boundary_side);
    if (boundary_side_map != _subid_pairs_to_sides.end())
      for (const auto & [elem, side] : boundary_side_map->second)
        boundary_info.add_side(elem, side, boundary_id);
    new_boundaryID++;
  }

  // Sync boundary info
  boundary_info.parallel_sync_side_ids();
  boundary_info.parallel_sync_node_ids();

  // Generate boundary_id pairs mapping based on _subid_pairs_to_sides
  for (const auto & [sub_pair, boundary_id] : _subid_pairs_to_boundary_id)
  {
    const auto rev_pair = std::make_pair(sub_pair.second, sub_pair.first);
    const bool has_reverse =
        _subid_pairs_to_boundary_id.find(rev_pair) != _subid_pairs_to_boundary_id.end();
    if (has_reverse)
      // Normal disconnected boundary pair: blockA_blockB <-> blockB_blockA
      mesh.add_disjoint_neighbor_boundary_pairs(
          boundary_id, _subid_pairs_to_boundary_id[rev_pair], RealVectorValue(0.0, 0.0, 0.0));
  }
}

subdomain_id_type
BreakMeshByBlockGenerator::blockRestrictedElementSubdomainID(const Elem * elem)
{
  subdomain_id_type elem_subdomain_id = elem->subdomain_id();
  if ((_block_pairs_restricted || _surrounding_blocks_restricted) &&
      (_block_set.find(elem_subdomain_id) == _block_set.end()))
    elem_subdomain_id = Elem::invalid_subdomain_id;

  return elem_subdomain_id;
}

bool
BreakMeshByBlockGenerator::findBlockPairs(subdomain_id_type block_one, subdomain_id_type block_two)
{
  for (auto block_pair : _block_pairs)
    if ((block_pair.first == block_one && block_pair.second == block_two) ||
        (block_pair.first == block_two && block_pair.second == block_one))
      return true;
  return false;
}

std::unordered_map<dof_id_type, std::set<subdomain_id_type>>
BreakMeshByBlockGenerator::syncConnectedBlocks(
    const std::unordered_map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map,
    const MeshBase & mesh)
{
  std::unordered_map<dof_id_type, std::set<subdomain_id_type>> nodeid_to_connected_blocks;

  // Phase 0: Each rank computes its local connected blocks for nodes it holds
  for (const auto & [node_id, elem_ids] : node_to_elem_map)
  {
    std::set<subdomain_id_type> connected_blocks;
    for (const dof_id_type elem_id : elem_ids)
    {
      const Elem * current_elem = mesh.elem_ptr(elem_id);
      subdomain_id_type block_id = blockRestrictedElementSubdomainID(current_elem);
      nodeid_to_connected_blocks[node_id].insert(block_id);
    }
  }

  // No synchronization needed for replicated meshes
  if (mesh.is_replicated())
    return nodeid_to_connected_blocks;

  auto & comm = mesh.comm();
  const auto mesh_pid = mesh.processor_id();

  // Phase 1: Ghost nodes push their connected blocks to the owner

  // (node_id, connected_blocks, ghost_pid)
  using NodeConnectedBlocksTuple =
      std::tuple<dof_id_type, std::vector<subdomain_id_type>, processor_id_type>;
  std::map<processor_id_type, std::vector<NodeConnectedBlocksTuple>> to_owner;
  for (const auto & [node_id, blocks] : nodeid_to_connected_blocks)
  {
    const Node * node = mesh.node_ptr(node_id);
    if (node && node->processor_id() != mesh_pid)
      to_owner[node->processor_id()].emplace_back(
          node_id, std::vector<subdomain_id_type>(blocks.begin(), blocks.end()), mesh_pid);
  }

  // Owner receives and merges ghost info into its local map, and tracks subscribers
  std::unordered_map<dof_id_type, std::set<processor_id_type>> subscribers;
  Parallel::push_parallel_vector_data(
      comm,
      to_owner,
      [&](processor_id_type, const std::vector<NodeConnectedBlocksTuple> & recv_data)
      {
        for (const auto & [node_id, blocks_vec, ghost_pid] : recv_data)
        {
          subscribers[node_id].insert(ghost_pid);
          nodeid_to_connected_blocks[node_id].insert(blocks_vec.begin(), blocks_vec.end());
        }
      });

  // Phase 2: Node owners broadcast complete connected blocks back to subscribers
  std::map<processor_id_type, std::vector<NodeConnectedBlocksPair>> from_owner;
  for (const auto & [node_id, sub_pids] : subscribers)
  {
    const Node * node = mesh.node_ptr(node_id);
    if (node && node->processor_id() == mesh_pid)
    {
      const auto & blocks_set = libmesh_map_find(nodeid_to_connected_blocks, node_id);
      for (const auto pid : sub_pids)
        from_owner[pid].emplace_back(
            node_id, std::vector<subdomain_id_type>(blocks_set.begin(), blocks_set.end()));
    }
  }

  Parallel::push_parallel_vector_data(
      comm,
      from_owner,
      [&](processor_id_type, const std::vector<NodeConnectedBlocksPair> & recv_data)
      {
        for (const auto & [node_id, blocks_vec] : recv_data)
          nodeid_to_connected_blocks[node_id].insert(blocks_vec.begin(), blocks_vec.end());
      });
  return nodeid_to_connected_blocks;
}
