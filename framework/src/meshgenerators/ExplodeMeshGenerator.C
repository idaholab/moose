//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplodeMeshGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/partitioner.h"

registerMooseObject("MooseApp", ExplodeMeshGenerator);

InputParameters
ExplodeMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Break all element-element interfaces in the specified subdomains.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<SubdomainID>>("subdomains", "The list of subdomain IDs to explode.");
  params.addRequiredParam<BoundaryName>(
      "interface_name", "The boundary name containing all broken element-element interfaces.");
  return params;
}

ExplodeMeshGenerator::ExplodeMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _subdomains(getParam<std::vector<SubdomainID>>("subdomains")),
    _interface_name(getParam<BoundaryName>("interface_name"))
{
}

std::unique_ptr<MeshBase>
ExplodeMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // check that the subdomain IDs exist in the mesh
  for (const auto & id : _subdomains)
    if (!MooseMeshUtils::hasSubdomainID(*mesh, id))
      paramError("subdomains", "The block ID '", id, "' was not found in the mesh");

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  if (boundary_info.get_id_by_name(_interface_name) != Moose::INVALID_BOUNDARY_ID)
    paramError("interface_name", "The specified interface name already exits in the mesh.");

  const auto node_to_elem_map = buildSubdomainRestrictedNodeToElemMap(mesh, _subdomains);

  duplicateNodes(mesh, node_to_elem_map);

  createInterface(*mesh, node_to_elem_map);

  Partitioner::set_node_processor_ids(*mesh);

  return dynamic_pointer_cast<MeshBase>(mesh);
}

ExplodeMeshGenerator::NodeToElemMapType
ExplodeMeshGenerator::buildSubdomainRestrictedNodeToElemMap(
    std::unique_ptr<MeshBase> & mesh, const std::vector<SubdomainID> & subdomains) const
{
  NodeToElemMapType node_to_elem_map;
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    // Skip if the element is not in the specified subdomains
    if (std::find(subdomains.begin(), subdomains.end(), elem->subdomain_id()) == subdomains.end())
      continue;

    std::set<const Elem *> neighbors;
    elem->find_point_neighbors(neighbors);

    for (auto n : make_range(elem->n_nodes()))
    {
      // if ANY neighboring element that contains this node is not in the specified subdomains,
      // don't add this node to the map, i.e. don't split this node.
      bool should_duplicate = true;
      for (auto neighbor : neighbors)
        if (neighbor->contains_point(elem->node_ref(n)) &&
            std::find(subdomains.begin(), subdomains.end(), neighbor->subdomain_id()) ==
                subdomains.end())
        {
          should_duplicate = false;
          break;
        }

      if (should_duplicate)
        node_to_elem_map[elem->node_id(n)].insert(elem->id());
    }
  }

  return node_to_elem_map;
}

void
ExplodeMeshGenerator::duplicateNodes(std::unique_ptr<MeshBase> & mesh,
                                     const NodeToElemMapType & node_to_elem_map) const
{
  for (const auto & [node_id, connected_elem_ids] : node_to_elem_map)
    for (auto & connected_elem_id : connected_elem_ids)
      if (connected_elem_id != *connected_elem_ids.begin())
        duplicateNode(mesh, mesh->elem_ptr(connected_elem_id), mesh->node_ptr(node_id));
}

void
ExplodeMeshGenerator::duplicateNode(std::unique_ptr<MeshBase> & mesh,
                                    Elem * elem,
                                    const Node * node) const
{
  std::unique_ptr<Node> new_node = Node::build(*node, Node::invalid_id);
  new_node->processor_id() = elem->processor_id();
  Node * added_node = mesh->add_node(std::move(new_node));
  for (const auto j : elem->node_index_range())
    if (elem->node_id(j) == node->id())
    {
      elem->set_node(j) = added_node;
      break;
    }

  // Add boundary info to the new node
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  std::vector<boundary_id_type> node_boundary_ids;
  boundary_info.boundary_ids(node, node_boundary_ids);
  boundary_info.add_node(added_node, node_boundary_ids);
}

void
ExplodeMeshGenerator::createInterface(MeshBase & mesh,
                                      const NodeToElemMapType & node_to_elem_map) const
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  const auto & existing_boundary_ids = boundary_info.get_boundary_ids();
  boundary_id_type interface_id =
      existing_boundary_ids.empty() ? 0 : *existing_boundary_ids.rbegin() + 1;
  boundary_info.sideset_name(interface_id) = _interface_name;

  std::set<std::pair<dof_id_type, unsigned int>> sides_to_add;

  for (const auto & node_to_elems : node_to_elem_map)
    for (const auto & elem_id_i : node_to_elems.second)
    {
      Elem * elem_i = mesh.elem_ptr(elem_id_i);
      for (const auto & elem_id_j : node_to_elems.second)
      {
        Elem * elem_j = mesh.elem_ptr(elem_id_j);
        if (elem_i != elem_j && elem_id_i < elem_id_j && elem_i->has_neighbor(elem_j))
          sides_to_add.insert(std::make_pair(elem_id_i, elem_i->which_neighbor_am_i(elem_j)));
      }
    }

  for (const auto & [elem_id, side] : sides_to_add)
    boundary_info.add_side(elem_id, side, interface_id);
}
