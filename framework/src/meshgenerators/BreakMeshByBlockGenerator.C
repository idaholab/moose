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
}

std::unique_ptr<MeshBase>
BreakMeshByBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  const unique_id_type n_total_nodes = mesh->parallel_n_nodes();

  std::set<boundary_id_type> ids = mesh->boundary_info->get_boundary_ids();
  _boundary_id_offset = *ids.rbegin() + 1;
  if (!mesh->is_replicated())
    mesh->comm().max(_boundary_id_offset);

  Moose::out << "OUT_ELEMENT_NODE_INITIAL_START\n";
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    Moose::out << elem->id() << " ";
    for (unsigned int node_id = 0; node_id < elem->n_nodes(); ++node_id)
      Moose::out << elem->node_id(node_id) << " ";
    Moose::out << std::endl;
  }
  Moose::out << "OUT_ELEMENT_NODE_INITIAL_END\n\n";

  Moose::out << "OUT_ELEMENT_SUBDOMAIN_START\n";
  for (const auto & elem : mesh->active_element_ptr_range())
    Moose::out << elem->id() << " " << elem->subdomain_id() << std::endl;
  Moose::out << "OUT_ELEMENT_SUBDOMAIN_END\n\n";

  Moose::out << "OUT_ELEMENT_PID_START\n";
  for (const auto & elem : mesh->active_element_ptr_range())
    Moose::out << elem->id() << " " << elem->processor_id() << std::endl;
  Moose::out << "OUT_ELEMENT_PID_END\n\n";

  Moose::out << "OUT_NODE_PID_INITIAL_START\n";
  for (const auto & node : mesh->node_ptr_range())
    Moose::out << node->id() << " " << node->processor_id() << std::endl;
  Moose::out << "OUT_NODE_PID_INITIAL_END\n\n";

  // initialize the node to element map
  std::map<dof_id_type, std::vector<dof_id_type>> node_to_elem_map;
  for (const auto & elem : mesh->active_element_ptr_range())
    if (elem != nullptr)
      for (unsigned int n = 0; n < elem->n_nodes(); n++)
        node_to_elem_map[elem->node_id(n)].push_back(elem->id());

  // construct the node to subdomain_pid map <node, <subdomain_id_type>
  std::map<dof_id_type, std::map<subdomain_id_type, processor_id_type>> node_to_domains_pid_map;
  std::map<subdomain_id_type, processor_id_type> connected_blocks_pid;

  for (const auto & node_it : node_to_elem_map)
  {
    const dof_id_type current_node_id = node_it.first;
    const Node * current_node = mesh->query_node_ptr(current_node_id);
    if (current_node != nullptr)
    {
      // find connected blocks
      connected_blocks_pid.clear();
      for (const auto & elem_id : node_it.second)
      {
        const Elem * current_elem = mesh->query_elem_ptr(elem_id);
        if (current_elem != nullptr)
        {
          subdomain_id_type elem_sub_id = current_elem->subdomain_id();
          processor_id_type elem_proc_id = current_elem->processor_id();
          auto it = connected_blocks_pid.find(elem_sub_id);
          if (it != connected_blocks_pid.end() && it->second > elem_proc_id)
            it->second = elem_proc_id;
          else if (it == connected_blocks_pid.end())
            connected_blocks_pid[elem_sub_id] = elem_proc_id;
        }
      }
      if (connected_blocks_pid.size() > 1) // add set do node to domain map
        node_to_domains_pid_map[current_node_id] = connected_blocks_pid;
    }
  } // once we are done with this we want to make sure boundaires id are the same

  // if the mesh is distributed do some communication to merge the node_to_domains_pid_map
  if (!mesh->is_replicated())
  {
    /* need to do communication
     we can only use vectors and vectors of vectors for communication so we start unrolling the
     node_to_domains_pid_map */
    std::map<processor_id_type, std::vector<dof_id_type>> data_to_send, received_data;

    std::vector<dof_id_type> flattened_data;
    // faltten data to send
    for (auto it_node : node_to_domains_pid_map)
      for (auto it_sub : it_node.second)
      {
        flattened_data.push_back(it_node.first);
        flattened_data.push_back(it_sub.first);
        flattened_data.push_back(it_sub.second);
      }

    // we want to send the data of each cpu to all other cpus so we will have a fully dense map
    for (processor_id_type i = 0; i < mesh->n_processors(); i++)
      data_to_send[i] = flattened_data;

    // compose replies from each cpu
    auto gather_data = [&received_data](processor_id_type pid,
                                        const std::vector<dof_id_type> & query) {
      received_data[pid] = query;
    };

    Parallel::push_parallel_vector_data(mesh->comm(), data_to_send, gather_data);

    // recreate the map
    node_to_domains_pid_map.clear();
    for (auto const & x : received_data)
      for (unsigned int k = 0; k < x.second.size(); k += 3)
      {
        dof_id_type n_id = x.second[k];
        subdomain_id_type s_id = x.second[k + 1];
        processor_id_type p_id = x.second[k + 2];
        // std::map<subdomain_id_type, processor_id_type> minimap = {{s_id, p_id}};
        auto n_it = node_to_domains_pid_map.find(n_id);
        if (n_it == node_to_domains_pid_map.end())
          node_to_domains_pid_map[n_id] = {{s_id, p_id}};
        else
        {
          auto s_it = n_it->second.find(s_id);
          if (s_it == n_it->second.end())
            n_it->second[s_id] = p_id;
          else if (s_it != n_it->second.end() && s_it->second > p_id)
            s_it->second = p_id;
        }
      }
  }

  // assign unique new node id and pid given subdomain
  // this map old_id->subddomain->(new_node_if, pid)
  std::map<dof_id_type, std::map<subdomain_id_type, std::pair<dof_id_type, processor_id_type>>>
      new_node_id_map;

  dof_id_type node_counter = n_total_nodes;
  for (auto node_it = node_to_domains_pid_map.begin(); node_it != node_to_domains_pid_map.end();
       ++node_it)
  {
    dof_id_type original_node_id = node_it->first;
    connected_blocks_pid = node_it->second;
    const subdomain_id_type first_id = connected_blocks_pid.begin()->first;

    connected_blocks_pid.erase(connected_blocks_pid.begin());
    for (auto sub_it : connected_blocks_pid)
    {
      new_node_id_map[original_node_id][sub_it.first] = std::make_pair(node_counter, sub_it.second);
      node_counter++;
      mooseAssert(first_id < sub_it.first, "first_id > sub_it.first");
      _neighboring_block_list.insert(std::make_pair(first_id, sub_it.first));
    }
  } // done creating new unique node ids

  // start adding nodes using the new unique identified ids
  for (auto node_it = new_node_id_map.begin(); node_it != new_node_id_map.end(); ++node_it)
  {
    dof_id_type old_node_id = node_it->first;
    const Node * old_node = mesh->query_node_ptr(old_node_id);
    if (old_node != nullptr)
    {
      auto connected_elem = node_to_elem_map.find(old_node_id);
      std::vector<boundary_id_type> node_boundary_ids;
      mesh->boundary_info->boundary_ids(old_node, node_boundary_ids);
      for (auto sub_dofid_pid_it : node_it->second)
      {
        subdomain_id_type sub_id = sub_dofid_pid_it.first;
        dof_id_type new_node_id = sub_dofid_pid_it.second.first;
        processor_id_type new_node_pid = sub_dofid_pid_it.second.second;

        Node * new_node = nullptr;
        new_node = Node::build(*old_node, DofObject::invalid_id).release();
        new_node->set_id() = new_node_id;
#ifdef LIBMESH_ENABLE_UNIQUE_ID
        new_node->set_unique_id() = new_node_id;
#endif
        // new_node->processor_id() = old_node->processor_id();
        new_node->processor_id() = new_node_pid;
        mesh->add_node(new_node);

        mesh->boundary_info->add_node(new_node, node_boundary_ids);
        for (auto elem_id : connected_elem->second)
        {
          Elem * elem = mesh->query_elem_ptr(elem_id);
          if (elem != nullptr)
            if (elem->subdomain_id() == sub_id)
            {
              mooseAssert(elem->subdomain_id() > 0, "elem->subdomain_id() <= 0");
              for (unsigned int node_id = 0; node_id < elem->n_nodes(); node_id++)
                if (elem->node_id(node_id) == old_node_id)
                  elem->set_node(node_id) = new_node;
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
              std::pair<subdomain_id_type, subdomain_id_type> blocks_pair = std::make_pair(
                  current_elem_pt->subdomain_id(), connected_elem_pt->subdomain_id());
              _new_boundary_sides_map[blocks_pair].insert(std::make_pair(
                  current_elem_pt->id(), current_elem_pt->which_neighbor_am_i(connected_elem_pt)));
            }
        }
    }
  } // done adding nodes and generating boundary block pairs

  // now we need a unique boundary id list

  Moose::out << "OUT_NODE_PID_START\n";
  for (const auto & node : mesh->node_ptr_range())
    Moose::out << node->id() << " " << node->processor_id() << std::endl;
  Moose::out << "OUT_NODE_PID_END\n\n";

  Moose::out << "OUT_ELEMENT_NODE_START\n";
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    Moose::out << elem->id() << " ";
    for (unsigned int node_id = 0; node_id < elem->n_nodes(); ++node_id)
      Moose::out << elem->node_id(node_id) << " ";
    Moose::out << std::endl;
  }
  Moose::out << "OUT_ELEMENT_NODE_END\n\n";

  addInterfaceBoundary(*mesh);
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
BreakMeshByBlockGenerator::addInterfaceBoundary(MeshBase & mesh)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  boundary_id_type boundaryID = _boundary_id_offset;

  std::string boundaryName = _interface_name;

  // loop over boundary sides
  for (auto & boundary_side_map : _new_boundary_sides_map)
  {

    // find the appropriate boundary name and id
    //  given master and slave block ID
    if (_split_interface)
    {
      boundaryName =
          generateBoundaryName(mesh, boundary_side_map.first.first, boundary_side_map.first.second);
      boundaryID = _boundary_id_offset +
                   std::distance(_neighboring_block_list.begin(),
                                 _neighboring_block_list.find(boundary_side_map.first));
      // findBoundaryNameAndInd(mesh,
      //                        boundary_side_map.first.first,
      //                        boundary_side_map.first.second,
      //                        boundaryName,
      //                        boundaryID,
      //                        boundary_info);
    }

    boundary_info.sideset_name(boundaryID) = boundaryName;

    // loop over all the side belonging to each pair and add it to the proper interface
    for (auto & element_side : boundary_side_map.second)
      boundary_info.add_side(element_side.first, element_side.second, boundaryID);
  }
}
