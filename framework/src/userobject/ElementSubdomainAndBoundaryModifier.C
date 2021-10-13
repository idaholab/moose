//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSubdomainAndBoundaryModifier.h"

#include "libmesh/remote_elem.h"

InputParameters
ElementSubdomainAndBoundaryModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addClassDescription("Modify element subdomain ID, and optionally a mesh boundary. This "
                             "userobject only runs on the undisplaced mesh, and it will "
                             "modify both the undisplaced and the displaced mesh.");
  params.addParam<BoundaryName>(
      "moving_boundary_name",
      "Optional boundary to modify when an element is moved. A boundary "
      "with the provided name will be created if it does not already exist");
  return params;
}

ElementSubdomainAndBoundaryModifier::ElementSubdomainAndBoundaryModifier(
    const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _moving_boundary_specified(isParamValid("moving_boundary_name"))
{
}

void
ElementSubdomainAndBoundaryModifier::initialSetup()
{
  if (_moving_boundary_specified)
  {
    _moving_boundary_name = getParam<BoundaryName>("moving_boundary_name");
    setMovingBoundaryName(_mesh);
    if (_displaced_problem)
      setMovingBoundaryName(_displaced_problem->mesh());
  }
}

void
ElementSubdomainAndBoundaryModifier::setMovingBoundaryName(MooseMesh & mesh)
{
  // We only need one boundary to modify. Create a dummy vector just to use the API.
  const std::vector<BoundaryID> boundary_ids = mesh.getBoundaryIDs({{_moving_boundary_name}}, true);
  mooseAssert(boundary_ids.size() == 1, "Expect exactly one boundary ID.");
  _moving_boundary_id = boundary_ids[0];
  mesh.setBoundaryName(_moving_boundary_id, _moving_boundary_name);
  mesh.getMesh().get_boundary_info().sideset_name(_moving_boundary_id) = _moving_boundary_name;
  mesh.getMesh().get_boundary_info().nodeset_name(_moving_boundary_id) = _moving_boundary_name;
}

void
ElementSubdomainAndBoundaryModifier::updateBoundaryInfo(
    MooseMesh & mesh, const std::vector<const Elem *> & moved_elems)
{
  if (!_moving_boundary_specified)
    return;

  BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();

  // save the removed ghost sides and associated nodes to sync across processors
  std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned int>>>
      ghost_sides_to_remove;
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> ghost_nodes_to_remove;

  // The logic below updates the side set and the node set associated with the moving boundary.
  std::set<dof_id_type> added_nodes, removed_nodes;
  for (auto elem : moved_elems)
  {
    // First loop over all the sides of the element
    for (auto side : elem->side_index_range())
    {
      const Elem * neighbor = elem->neighbor_ptr(side);
      if (neighbor && neighbor != libMesh::remote_elem)
      {
        // If the neighbor has a different subdomain ID, then this side should be added to
        // the moving boundary
        if (neighbor->subdomain_id() != elem->subdomain_id())
          bnd_info.add_side(elem, side, _moving_boundary_id);
        // Otherwise remove this side and the neighbor side from the boundary.
        else
        {
          bnd_info.remove_side(elem, side);
          unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);
          bnd_info.remove_side(neighbor, neighbor_side);
          if (neighbor->processor_id() != this->processor_id())
            ghost_sides_to_remove[neighbor->processor_id()].emplace_back(neighbor->id(),
                                                                         neighbor_side);
        }
      }
    }

    // Then loop over all the nodes of the element
    for (auto node : elem->node_index_range())
    {
      // Find the point neighbors
      std::set<const Elem *> neighbor_set;
      elem->find_point_neighbors(elem->node_ref(node), neighbor_set);
      for (auto neighbor : neighbor_set)
        if (neighbor != libMesh::remote_elem)
        {
          // If the neighbor has a different subdomain ID, then this node should be added to
          // the moving boundary
          if (neighbor->subdomain_id() != elem->subdomain_id())
            added_nodes.insert(elem->node_id(node));
          // Otherwise remove this node from the boundary.
          else
          {
            removed_nodes.insert(elem->node_id(node));
            if (neighbor->processor_id() != this->processor_id())
              ghost_nodes_to_remove[neighbor->processor_id()].push_back(elem->node_id(node));
          }
        }
    }
  }

  // make sure to remove nodes that are not in the add set
  std::set<dof_id_type> nodes_to_remove;
  std::set_difference(removed_nodes.begin(),
                      removed_nodes.end(),
                      added_nodes.begin(),
                      added_nodes.end(),
                      std::inserter(nodes_to_remove, nodes_to_remove.end()));
  for (auto node_id : nodes_to_remove)
    mesh.getMesh().get_boundary_info().remove_node(mesh.nodePtr(node_id), _moving_boundary_id);
  // synchronize boundary information across processors
  pushBoundarySideInfo(mesh, ghost_sides_to_remove);
  pushBoundaryNodeInfo(mesh, ghost_nodes_to_remove);
  mesh.getMesh().get_boundary_info().parallel_sync_side_ids();
  mesh.getMesh().get_boundary_info().parallel_sync_node_ids();
  mesh.update();
}

void
ElementSubdomainAndBoundaryModifier::pushBoundarySideInfo(
    MooseMesh & mesh,
    std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned int>>> &
        elems_to_push)
{
  auto elem_action_functor =
      [&mesh, this](processor_id_type,
                    const std::vector<std::pair<dof_id_type, unsigned int>> & received_elem) {
        // remove the side
        for (const auto & pr : received_elem)
          mesh.getMesh().get_boundary_info().remove_side(
              mesh.getMesh().elem_ptr(pr.first), pr.second, _moving_boundary_id);
      };

  Parallel::push_parallel_vector_data(
      mesh.getMesh().get_boundary_info().comm(), elems_to_push, elem_action_functor);
}

void
ElementSubdomainAndBoundaryModifier::pushBoundaryNodeInfo(
    MooseMesh & mesh,
    std::unordered_map<processor_id_type, std::vector<dof_id_type>> & nodes_to_push)
{
  auto node_action_functor = [&mesh, this](processor_id_type,
                                           const std::vector<dof_id_type> & received_nodes) {
    for (const auto & pr : received_nodes)
      mesh.getMesh().get_boundary_info().remove_node(mesh.getMesh().node_ptr(pr),
                                                     _moving_boundary_id);
  };

  Parallel::push_parallel_vector_data(
      mesh.getMesh().get_boundary_info().comm(), nodes_to_push, node_action_functor);
}
