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
#include "libmesh/parallel_sync.h"
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
  // if (typeid(_input).name() == typeid(DistributedMesh).name())
  //   mooseError("BreakMeshByBlockGenerator only works with ReplicatedMesh.");
}

std::unique_ptr<MeshBase>
BreakMeshByBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  const unique_id_type n_total_nodes = mesh->parallel_n_nodes();
  std::cout << "INTIAL NODE SETUP \n";
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    std::cout << "Rank " << mesh->processor_id() << " elem_id " << elem->id() << " ";
    for (unsigned int node_id = 0; node_id < elem->n_nodes(); ++node_id)
      std::cout << "node" << elem->node_id(node_id) << " ";

    std::cout << " nn " << elem->n_neighbors() << "\n";
  }

  // initialize the node to element map
  std::map<dof_id_type, std::vector<dof_id_type>> node_to_elem_map;
  for (const auto & elem : mesh->active_element_ptr_range())
    if (elem != nullptr)
      for (unsigned int n = 0; n < elem->n_nodes(); n++)
        node_to_elem_map[elem->node_id(n)].push_back(elem->id());

  // construct the node to domain map <node, subdomain_id_type>
  std::map<dof_id_type, std::set<subdomain_id_type>> node_to_domains_map;
  std::set<subdomain_id_type> connected_blocks;

  for (const auto & node_it : node_to_elem_map)
  {
    const dof_id_type current_node_id = node_it.first;
    const Node * current_node = mesh->query_node_ptr(current_node_id);
    if (current_node != nullptr)
    {
      // find connected blocks
      connected_blocks.clear();
      for (const auto & elem_id : node_it.second)
      {
        const Elem * current_elem = mesh->query_elem_ptr(elem_id);
        if (current_elem != nullptr)
          connected_blocks.insert(current_elem->subdomain_id());
      }
      if (connected_blocks.size() > 1) // add set do node to domain map
        node_to_domains_map[current_node_id] = connected_blocks;
    }
  } // once we are doine with this we want to make sure boundaires id are the same

  // if the mesh is distributed do some communication to merge the node_to_domains_map
  if (!mesh->is_replicated())
  {
    /* need to do communication
     we can only use vectors and vectors of vectors for communication so we start unrolling the
     node_to_domains_map */
    std::map<processor_id_type, std::vector<dof_id_type>> data_to_send, received_data;

    std::vector<dof_id_type> flattened_data;
    // faltten data to send
    for (auto it_node : node_to_domains_map)
      for (auto it_sub : it_node.second)
      {
        flattened_data.push_back(it_node.first);
        flattened_data.push_back(it_sub);
      }

    // we want to send the data of each cpu to all other cpus so we will have a fully dense map
    for (processor_id_type i = 0; i < mesh->n_processors(); i++)
      data_to_send[i] = flattened_data;

    // compose replies from each cpu
    auto gather_data = [&received_data](processor_id_type pid,
                                        const std::vector<dof_id_type> & query) {
      // response = query;
      received_data[pid] = query;
    };

    Parallel::push_parallel_vector_data(mesh->comm(), data_to_send, gather_data);

    // recreate the map
    node_to_domains_map.clear();
    for (auto const & x : received_data)
      for (unsigned int k = 0; k < x.second.size(); k += 2)
        node_to_domains_map[x.second[k]].insert(x.second[k + 1]);
  }

  // assign unique new node ids based on domain id
  std::map<dof_id_type, std::vector<std::pair<subdomain_id_type, dof_id_type>>> new_node_id_map;
  dof_id_type node_counter = n_total_nodes;
  for (auto node_it = node_to_domains_map.begin(); node_it != node_to_domains_map.end(); ++node_it)
  {
    dof_id_type original_node_id = node_it->first;
    auto connected_blocks = node_it->second;
    connected_blocks.erase(connected_blocks.begin());
    for (auto sub_it : connected_blocks)
    {
      new_node_id_map[original_node_id].push_back(std::make_pair(sub_it, node_counter));
      node_counter++;
    }
  } // done creating new unique node ids

  // start adding nodes giving the new identified ids
  for (auto node_it = new_node_id_map.begin(); node_it != new_node_id_map.end(); ++node_it)
  {
    dof_id_type old_node_id = node_it->first;
    const Node * old_node = mesh->query_node_ptr(old_node_id);
    if (old_node != nullptr)
    {
      auto connected_elem = node_to_elem_map.find(old_node_id);
      std::vector<boundary_id_type> node_boundary_ids;
      mesh->boundary_info->boundary_ids(old_node, node_boundary_ids);
      for (auto sub_node : node_it->second)
      {
        Node * new_node = nullptr;
        new_node = Node::build(*old_node, DofObject::invalid_id).release();
        new_node->set_id() = sub_node.second;
#ifdef LIBMESH_ENABLE_UNIQUE_ID
        new_node->set_unique_id() = sub_node.second;
#endif
        new_node->processor_id() = old_node->processor_id();
        mesh->add_node(new_node);

        mesh->boundary_info->add_node(new_node, node_boundary_ids);
        for (auto elem_id : connected_elem->second)
        {
          Elem * elem = mesh->query_elem_ptr(elem_id);
          if (elem != nullptr)
            if (elem->subdomain_id() == sub_node.first)
            {
              for (unsigned int node_id = 0; node_id < elem->n_nodes(); node_id++)
                if (elem->node_id(node_id) == old_node_id)
                  elem->set_node(node_id) = new_node;

              // std::cout << "Rank " << mesh->processor_id() << " elem_id " << elem_id << " ";
              // for (unsigned int node_id = 0; node_id < elem->n_nodes(); ++node_id)
              //   std::cout << "node" << elem->node_id(node_id) << " ";
              //
              // std::cout << " nn " << elem->n_neighbors() << "\n";
            }
        }
      }

      // create blocks pair and assign element side to new interface boundary map

      for (auto elem_id : connected_elem->second)
        for (auto connected_elem_id : connected_elem->second)
        {
          Elem * current_elem_pt = mesh->query_elem_ptr(elem_id);
          Elem * connected_elem_pt = mesh->query_elem_ptr(connected_elem_id);

          if ((current_elem_pt != nullptr) && (connected_elem_pt != nullptr) &&
              (current_elem_pt != connected_elem_pt) &&
              (current_elem_pt->subdomain_id() < connected_elem_pt->subdomain_id()))
            if (current_elem_pt->has_neighbor(connected_elem_pt))
            {
              // std::cout << "Rank " << mesh->processor_id() << "  curr_elem id "
              //           << current_elem_pt->id() << "  connected_elem id "
              //           << connected_elem_pt->id() << "  neigh curr_conn"
              //           << current_elem_pt->which_neighbor_am_i(connected_elem_pt)
              //           << "  neigh conn_curr"
              //           << connected_elem_pt->which_neighbor_am_i(current_elem_pt) << std::endl;
              // std::pair<subdomain_id_type, subdomain_id_type> blocks_pair = std::make_pair(
              //     current_elem_pt->subdomain_id(), connected_elem_pt->subdomain_id());
              //
              // _new_boundary_sides_map[blocks_pair].insert(std::make_pair(
              //     current_elem_pt->id(),
              //     current_elem_pt->which_neighbor_am_i(connected_elem_pt)));
            }
        }
    }
  }

  std::cout << "FINAL NODE SETUP \n";
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    std::cout << "Rank " << mesh->processor_id() << " elem_id " << elem->id() << " nodes";
    for (unsigned int node_id = 0; node_id < elem->n_nodes(); ++node_id)
      std::cout << elem->node_id(node_id) << " ";

    std::cout << " n_neighbors " << elem->n_neighbors() << " n_sides " << elem->n_sides()
              << " neighbor ";
    for (const auto & conn_elem : mesh->active_element_ptr_range())
      if (elem->has_neighbor(conn_elem))
        std::cout << "  connelem_id " << conn_elem->id() << "   wNamI "
                  << elem->which_neighbor_am_i(conn_elem) << "  "
                  << conn_elem->which_neighbor_am_i(elem) << "; ";

    std::cout << std::endl;
  }

  // for (const auto & elem : mesh->active_element_ptr_range())
  //   if (elem != nullptr)
  //     for (const auto & conn_elem : mesh->active_element_ptr_range())
  //       if (conn_elem != nullptr && elem->has_neighbor(conn_elem))
  //         std::cout << "Rank " << mesh->processor_id() << "  curr_elem id " << elem->id()
  //                   << "  connected_elem id " << conn_elem->id() << "  neigh curr_conn"
  //                   << elem->which_neighbor_am_i(conn_elem) << "  neigh conn_curr"
  //                   << conn_elem->which_neighbor_am_i(elem) << std::endl;

  // addInterfaceBoundary(*mesh);
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
    //  given master and slave block ID
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
