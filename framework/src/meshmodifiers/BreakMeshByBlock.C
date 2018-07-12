//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlock.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", BreakMeshByBlock);

template <>
InputParameters
validParams<BreakMeshByBlock>()
{
  InputParameters params = validParams<BreakMeshByBlockBase>();
  params.addClassDescription("Break boundaries based on the subdomains to which their sides are "
                             "attached. Naming convention for the new boundaries will be the old "
                             "boundary name plus \"_to_\" plus the subdomain name. At the moment"
                             "this only works on REPLICATED mesh");
  return params;
}

BreakMeshByBlock::BreakMeshByBlock(const InputParameters & parameters)
  : BreakMeshByBlockBase(parameters)
{
}

void
BreakMeshByBlock::modify()
{

  // TODO remove when distributed MESH capabilities are implemented
  _mesh_ptr->errorIfDistributedMesh("BreakMeshByBlock only works on a REPLICATED mesh");
  checkInputParameter();

  // save reference to mesh
  MeshBase & mesh = _mesh_ptr->getMesh();

  // initialize the node to elemen map
  const auto & node_to_elem_map = _mesh_ptr->nodeToElemMap();

  for (auto node_it = node_to_elem_map.begin(); node_it != node_to_elem_map.end(); ++node_it)
  {
    const dof_id_type current_node_id = node_it->first;
    const Node * current_node = mesh.node_ptr(current_node_id);

    if (current_node != nullptr)
    {

      // find node multiplicity
      const std::set<SubdomainID> & connected_blocks = _mesh_ptr->getNodeBlockIds(*current_node);
      unsigned int node_multiplicity = connected_blocks.size();

      // check if current_node need to be duplicated
      if (node_multiplicity > 1)
      {

        // retrieve connected elements from the map
        const std::vector<dof_id_type> & connected_elems = node_it->second;

        // find reference_subdomain_id (e.g. the subdomain with lower id)
        auto subdomain_it = connected_blocks.begin();
        SubdomainID reference_subdomain_id = *subdomain_it;

        // we need to keep track of which element belong to the reference block and which
        // to neighboring blocks
        std::set<dof_id_type> reference_elems;
        std::set<dof_id_type> neighbor_elems;

        // initialize current multiplicity to keep track of how many nodes we added
        unsigned int current_multiplicity = 1;

        // loop over node multiplicity to add nodes starting from the second subdomain
        // the original node remains on the reference_subdoamin
        while (current_multiplicity < node_multiplicity)
        {

          // add new node
          Node * new_node = Node::build(*current_node, mesh.n_nodes()).release();
          new_node->processor_id() = current_node->processor_id();
          mesh.add_node(new_node);
          current_multiplicity += 1; // node created, update multiplicity

          // fix all the elements belonging to the connected_blocks[subdomain_it]
          for (auto elem_id : connected_elems)
          {
            Elem * current_elem = mesh.elem_ptr(elem_id);

            // check if we need to fix a node in the current element
            // if current_elem belong to the reference subdomain
            if (current_elem->subdomain_id() == reference_subdomain_id)
              reference_elems.insert(current_elem->id()); // store reference element
            else                                          // this is a neighbor element
            {
              // add the current element to the neighbor elements list
              neighbor_elems.insert(current_elem->id());

              // fix current element node:
              // cicle over the neighbor element nodes to find the node to fix
              for (unsigned int local_elem_node_it = 0;
                   local_elem_node_it < current_elem->n_nodes();
                   ++local_elem_node_it)
                if (current_elem->node_id(local_elem_node_it) ==
                    current_node->id()) // if current node == node on element
                {
                  current_elem->set_node(local_elem_node_it) = new_node;
                  break; // ones the proper node has been fixed in one element we can break the loop
                }
            } // end reference/neighbor actions
          }   // end loop over connected elements
        }     // end multiplicity loop

        // to add the appropriate side to the new interface we need to loop over
        // reference elements and find all the neighbors sharing a side with it
        for (auto ref_elem_id : reference_elems)
        {

          Elem * ref_elem = mesh.elem_ptr(ref_elem_id);

          // loop over possible neighbors
          for (auto neigh_elem_id : neighbor_elems)
          {
            Elem * neigh_elem = mesh.elem_ptr(neigh_elem_id);
            if (ref_elem->has_neighbor(neigh_elem))
            {
              std::pair<subdomain_id_type, subdomain_id_type> blocks_pair =
                  std::make_pair(ref_elem->subdomain_id(), neigh_elem->subdomain_id());

              _new_boundary_sides_map[blocks_pair].insert(
                  std::make_pair(ref_elem->id(), ref_elem->which_neighbor_am_i(neigh_elem)));
            }
          }
        }
      } // end multiplicity check
    }   // end loop over nodes
  }     // end nodeptr check

  addInterfaceBoundary();
}

void
BreakMeshByBlock::addInterfaceBoundary()
{
  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();

  BoundaryID boundaryID = findFreeBoundaryId();
  std::string boundaryName = _interface_name;

  // loop over boundary sides
  for (auto & boundary_side_map : _new_boundary_sides_map)
  {

    // find the appropriate boundary name and id
    //  given master and slave block ID
    if (_split_interface)
      findBoundaryNameAndInd(boundary_side_map.first.first,
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
