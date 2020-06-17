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

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", BreakMeshByBlockGenerator);

defineLegacyParams(BreakMeshByBlockGenerator);

InputParameters
BreakMeshByBlockGenerator::validParams()
{
  InputParameters params = BreakMeshByBlockGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Break boundaries based on the subdomains to which their sides are "
                             "attached. Naming convention for the new boundaries will be the old "
                             "boundary name plus \"_to_\" plus the subdomain name. At the moment"
                             "this only works on REPLICATED mesh");
  return params;
}

BreakMeshByBlockGenerator::BreakMeshByBlockGenerator(const InputParameters & parameters)
  : BreakMeshByBlockGeneratorBase(parameters), _input(getMesh("input"))
{
  if (typeid(_input).name() == typeid(DistributedMesh).name())
    mooseError("BreakMeshByBlockGenerator only works with ReplicatedMesh.");
}

std::unique_ptr<MeshBase>
BreakMeshByBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

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
        connected_blocks.insert(current_elem->subdomain_id());
      }

      unsigned int node_multiplicity = connected_blocks.size();

      // check if current_node need to be duplicated
      if (node_multiplicity > 1)
      {
        // retrieve connected elements from the map
        const std::vector<dof_id_type> & connected_elems = node_it->second;

        // find reference_subdomain_id (e.g. the subdomain with lower id)
        auto subdomain_it = connected_blocks.begin();
        subdomain_id_type reference_subdomain_id = *subdomain_it;

        // multiplicity counter to keep track of how many nodes we added
        unsigned int multiplicity_counter = node_multiplicity;
        for (auto elem_id : connected_elems)
        {
          // all the duplicate nodes are added and assigned
          if (multiplicity_counter == 0)
            break;

          Elem * current_elem = mesh->elem_ptr(elem_id);
          if (current_elem->subdomain_id() != reference_subdomain_id)
          {
            // assign the newly added node to current_elem
            Node * new_node = nullptr;

            std::vector<boundary_id_type> node_boundary_ids;

            for (unsigned int node_id = 0; node_id < current_elem->n_nodes(); ++node_id)
              if (current_elem->node_id(node_id) ==
                  current_node->id()) // if current node == node on element
              {
                // add new node
                new_node = Node::build(*current_node, mesh->n_nodes()).release();

                // We're duplicating nodes so that each subdomain elem has its own copy, so it seems
                // natural to assign this new node the same proc id as corresponding subdomain elem
                new_node->processor_id() = current_elem->processor_id();
                mesh->add_node(new_node);

                // Add boundary info to the new node
                mesh->boundary_info->boundary_ids(current_node, node_boundary_ids);
                mesh->boundary_info->add_node(new_node, node_boundary_ids);

                multiplicity_counter--; // node created, update multiplicity counter

                current_elem->set_node(node_id) = new_node;
                break; // ones the proper node has been fixed in one element we can break the
                       // loop
              }

            for (auto connected_elem_id : connected_elems)
            {
              Elem * connected_elem = mesh->elem_ptr(connected_elem_id);

              // Assign the newly added node to other connected elements with the same block_id
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

        // create blocks pair and assign element side to new interface boundary map
        for (auto elem_id : connected_elems)
        {
          for (auto connected_elem_id : connected_elems)
          {
            Elem * current_elem = mesh->elem_ptr(elem_id);
            Elem * connected_elem = mesh->elem_ptr(connected_elem_id);

            if (current_elem != connected_elem &&
                current_elem->subdomain_id() < connected_elem->subdomain_id())
            {
              if (current_elem->has_neighbor(connected_elem))
              {
                std::pair<subdomain_id_type, subdomain_id_type> blocks_pair =
                    std::make_pair(current_elem->subdomain_id(), connected_elem->subdomain_id());

                _new_boundary_sides_map[blocks_pair].insert(std::make_pair(
                    current_elem->id(), current_elem->which_neighbor_am_i(connected_elem)));
              }
            }
          }
        }

      } // end multiplicity check
    }   // end loop over nodes
  }     // end nodeptr check

  addInterfaceBoundary(*mesh);
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
BreakMeshByBlockGenerator::addInterfaceBoundary(MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  boundary_id_type boundaryID = findFreeBoundaryId(mesh);
  std::string boundaryName = _interface_name;

  // loop over boundary sides
  for (auto & boundary_side_map : _new_boundary_sides_map)
  {

    // find the appropriate boundary name and id
    //  given master and secondary block ID
    if (_split_interface)
      findBoundaryNameAndInd(mesh,
                             boundary_side_map.first.first,
                             boundary_side_map.first.second,
                             boundaryName,
                             boundaryID,
                             boundary_info);
    else
      boundary_info.sideset_name(boundaryID) = boundaryName;

    // loop over all the side belonging to each pair and add it to the proper interface
    for (auto & element_side : boundary_side_map.second)
      boundary_info.add_side(element_side.first, element_side.second, boundaryID);
  }
}
