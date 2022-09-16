//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include <typeinfo>

registerMooseObject("MooseApp", BreakMeshByBlockGenerator);

InputParameters
BreakMeshByBlockGenerator::validParams()
{
  InputParameters params = BreakMeshByBlockGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Break boundaries based on the subdomains to which their sides are "
                             "attached. Naming convention for the new boundaries will be the old "
                             "boundary name plus \"_to_\" plus the subdomain name. At the moment"
                             "this only works on REPLICATED mesh");
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
                        false,
                        "Whether to add an additional interface boundary at the other side.");
  params.addParam<BoundaryName>(
      "interface_transition_name",
      "interface_transition",
      "the name of the interface transition boundary created when blocks are provided");
  return params;
}

BreakMeshByBlockGenerator::BreakMeshByBlockGenerator(const InputParameters & parameters)
  : BreakMeshByBlockGeneratorBase(parameters),
    _input(getMesh("input")),
    _block_pairs_restricted(parameters.isParamSetByUser("block_pairs")),
    _surrounding_blocks_restricted(parameters.isParamSetByUser("surrounding_blocks")),
    _add_transition_interface(getParam<bool>("add_transition_interface")),
    _split_transition_interface(getParam<bool>("split_transition_interface")),
    _interface_transition_name(getParam<BoundaryName>("interface_transition_name")),
    _add_interface_on_two_sides(getParam<bool>("add_interface_on_two_sides"))
{
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

std::unique_ptr<MeshBase>
BreakMeshByBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseError("BreakMeshByBlockGenerator is not implemented for distributed meshes");

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
  std::map<dof_id_type, std::vector<dof_id_type>> node_to_elem_map;
  for (const auto & elem : mesh->active_element_ptr_range())
    for (unsigned int n = 0; n < elem->n_nodes(); n++)
      node_to_elem_map[elem->node_id(n)].push_back(elem->id());

  for (auto node_it = node_to_elem_map.begin(); node_it != node_to_elem_map.end(); ++node_it)
  {
    const dof_id_type current_node_id = node_it->first;
    const Node * current_node = mesh->node_ptr(current_node_id);

    if (current_node != nullptr)
    {
      // find node multiplicity
      std::set<subdomain_id_type> connected_blocks;
      for (auto elem_id = node_it->second.begin(); elem_id != node_it->second.end(); elem_id++)
      {
        const Elem * current_elem = mesh->elem_ptr(*elem_id);
        subdomain_id_type block_id = blockRestrictedElementSubdomainID(current_elem);
        if (!_block_pairs_restricted)
          connected_blocks.insert(block_id);
        else
        {
          if (block_id != Elem::invalid_subdomain_id)
            connected_blocks.insert(block_id);
        }
      }

      unsigned int node_multiplicity = connected_blocks.size();

      // check if current_node need to be duplicated
      if (node_multiplicity > 1)
      {
        // retrieve connected elements from the map
        const std::vector<dof_id_type> & connected_elems = node_it->second;

        // find reference_subdomain_id (e.g. the subdomain with lower id or the
        // Elem::invalid_subdomain_id)
        auto subdomain_it = connected_blocks.begin();
        subdomain_id_type reference_subdomain_id =
            connected_blocks.find(Elem::invalid_subdomain_id) != connected_blocks.end()
                ? Elem::invalid_subdomain_id
                : *subdomain_it;

        // For the block_pairs option, the node that is only shared by specific block pairs will be
        // newly created.
        bool should_create_new_node = true;
        if (_block_pairs_restricted)
        {
          auto elems = node_to_elem_map[current_node->id()];
          should_create_new_node = false;
          std::set<subdomain_id_type> sets_blocks_for_this_node;
          for (auto elem_id = elems.begin(); elem_id != elems.end(); elem_id++)
            sets_blocks_for_this_node.insert(
                blockRestrictedElementSubdomainID(mesh->elem_ptr(*elem_id)));
          if (sets_blocks_for_this_node.size() == 2)
          {
            auto setIt = sets_blocks_for_this_node.begin();
            if (findBlockPairs(*setIt, *std::next(setIt, 1)))
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

          if ((blockRestrictedElementSubdomainID(current_elem) != reference_subdomain_id) ||
              (_block_pairs_restricted & findBlockPairs(reference_subdomain_id, block_id)))
          {
            // assign the newly added node to current_elem
            Node * new_node = nullptr;

            std::vector<boundary_id_type> node_boundary_ids;

            for (unsigned int node_id = 0; node_id < current_elem->n_nodes(); ++node_id)
              if (current_elem->node_id(node_id) ==
                  current_node->id()) // if current node == node on element
              {
                if (should_create_new_node)
                {
                  // add new node
                  new_node = Node::build(*current_node, mesh->n_nodes()).release();

                  // We're duplicating nodes so that each subdomain elem has its own copy, so it
                  // seems natural to assign this new node the same proc id as corresponding
                  // subdomain elem
                  new_node->processor_id() = current_elem->processor_id();
                  mesh->add_node(new_node);
                  current_elem->set_node(node_id) = new_node;
                  // Add boundary info to the new node
                  boundary_info.boundary_ids(current_node, node_boundary_ids);
                  boundary_info.add_node(new_node, node_boundary_ids);
                }

                multiplicity_counter--; // node created, update multiplicity counter

                break; // ones the proper node has been fixed in one element we can break the
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
                  for (unsigned int node_id = 0; node_id < connected_elem->n_nodes(); ++node_id)
                    if (connected_elem->node_id(node_id) ==
                        current_node->id()) // if current node == node on element
                    {
                      connected_elem->set_node(node_id) = new_node;
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
            subdomain_id_type curr_elem_subid = blockRestrictedElementSubdomainID(current_elem);
            subdomain_id_type connected_elem_subid =
                blockRestrictedElementSubdomainID(connected_elem);

            if (current_elem != connected_elem && curr_elem_subid < connected_elem_subid)
            {
              if (current_elem->has_neighbor(connected_elem))
              {
                dof_id_type elem_id = current_elem->id();
                dof_id_type connected_elem_id = connected_elem->id();
                unsigned int side = current_elem->which_neighbor_am_i(connected_elem);
                unsigned int connected_elem_side =
                    connected_elem->which_neighbor_am_i(current_elem);

                // in this case we need to play a game to reorder the sides
                if (_block_pairs_restricted || _surrounding_blocks_restricted)
                {
                  connected_elem_subid = connected_elem->subdomain_id();
                  if (curr_elem_subid > connected_elem_subid) // we need to switch the ids
                  {
                    connected_elem_subid = current_elem->subdomain_id();
                    curr_elem_subid = connected_elem->subdomain_id();

                    elem_id = connected_elem->id();
                    side = connected_elem->which_neighbor_am_i(current_elem);

                    connected_elem_id = current_elem->id();
                    connected_elem_side = current_elem->which_neighbor_am_i(connected_elem);
                  }
                }

                std::pair<subdomain_id_type, subdomain_id_type> blocks_pair =
                    std::make_pair(curr_elem_subid, connected_elem_subid);

                std::pair<subdomain_id_type, subdomain_id_type> blocks_pair2 =
                    std::make_pair(connected_elem_subid, curr_elem_subid);

                if (_block_pairs_restricted)
                {
                  if (findBlockPairs(blockRestrictedElementSubdomainID(current_elem),
                                     blockRestrictedElementSubdomainID(connected_elem)))
                  {
                    _new_boundary_sides_map[blocks_pair].insert(std::make_pair(elem_id, side));
                    if (_add_interface_on_two_sides)
                      _new_boundary_sides_map[blocks_pair2].insert(
                          std::make_pair(connected_elem_id, connected_elem_side));
                  }
                }
                else
                {
                  _new_boundary_sides_map[blocks_pair].insert(std::make_pair(elem_id, side));
                  if (_add_interface_on_two_sides)
                    _new_boundary_sides_map[blocks_pair2].insert(
                        std::make_pair(connected_elem_id, connected_elem_side));
                }
              }
            }
          }
        }

      } // end multiplicity check
    }   // end loop over nodes
  }     // end nodeptr check

  addInterfaceBoundary(*mesh);
  Partitioner::set_node_processor_ids(*mesh);
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
BreakMeshByBlockGenerator::addInterfaceBoundary(MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  boundary_id_type boundary_id;
  boundary_id_type boundary_id_interface = Moose::INVALID_BOUNDARY_ID;
  boundary_id_type boundary_id_interface_transition = Moose::INVALID_BOUNDARY_ID;

  BoundaryName boundary_name;

  // loop over boundary sides
  for (auto & boundary_side_map : _new_boundary_sides_map)
  {
    if (!(_block_pairs_restricted || _surrounding_blocks_restricted) ||
        ((_block_pairs_restricted || _surrounding_blocks_restricted) && !_add_transition_interface))
    {
      // find the appropriate boundary name and id
      // given primary and secondary block ID
      if (_split_interface)
        findBoundaryNameAndInd(mesh,
                               boundary_side_map.first.first,
                               boundary_side_map.first.second,
                               boundary_name,
                               boundary_id,
                               boundary_info);
      else
      {
        boundary_name = _interface_name;
        boundary_id_interface = boundary_id_interface == Moose::INVALID_BOUNDARY_ID
                                    ? findFreeBoundaryId(mesh)
                                    : boundary_id_interface;
        boundary_id = boundary_id_interface;
        boundary_info.sideset_name(boundary_id_interface) = boundary_name;
      }
    }
    else // block resticted with transition boundary
    {

      const bool interior_boundary =
          _block_set.find(boundary_side_map.first.first) != _block_set.end() &&
          _block_set.find(boundary_side_map.first.second) != _block_set.end();

      if ((_split_interface && interior_boundary) ||
          (_split_transition_interface && !interior_boundary))
      {
        findBoundaryNameAndInd(mesh,
                               boundary_side_map.first.first,
                               boundary_side_map.first.second,
                               boundary_name,
                               boundary_id,
                               boundary_info);
      }
      else if (interior_boundary)
      {
        boundary_name = _interface_name;
        boundary_id_interface = boundary_id_interface == Moose::INVALID_BOUNDARY_ID
                                    ? findFreeBoundaryId(mesh)
                                    : boundary_id_interface;

        boundary_id = boundary_id_interface;
        boundary_info.sideset_name(boundary_id_interface) = boundary_name;
      }
      else
      {
        boundary_id_interface_transition =
            boundary_id_interface_transition == Moose::INVALID_BOUNDARY_ID
                ? findFreeBoundaryId(mesh)
                : boundary_id_interface_transition;
        boundary_id = boundary_id_interface_transition;
        boundary_info.sideset_name(boundary_id) = _interface_transition_name;
      }
    }
    // loop over all the side belonging to each pair and add it to the proper interface
    for (auto & element_side : boundary_side_map.second)
      boundary_info.add_side(element_side.first, element_side.second, boundary_id);
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
