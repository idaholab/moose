//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LowerDBlockFromSidesetGenerator.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/compare_elems_by_level.h"
#include "libmesh/mesh_communication.h"

#include "timpi/parallel_sync.h"

#include <set>
#include <typeinfo>

registerMooseObject("MooseApp", LowerDBlockFromSidesetGenerator);

InputParameters
LowerDBlockFromSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<SubdomainID>("new_block_id", "The lower dimensional block id to create");
  params.addParam<SubdomainName>("new_block_name",
                                 "The lower dimensional block name to create (optional)");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "sidesets", "The sidesets from which to create the new block");

  params.addClassDescription("Adds lower dimensional elements on the specified sidesets.");

  return params;
}

LowerDBlockFromSidesetGenerator::LowerDBlockFromSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _sideset_names(getParam<std::vector<BoundaryName>>("sidesets"))
{
}

// Used to temporarily store information about which lower-dimensional
// sides to add and what subdomain id to use for the added sides.
struct ElemSideDouble
{
  ElemSideDouble(Elem * elem_in, unsigned short int side_in) : elem(elem_in), side(side_in) {}

  Elem * elem;
  unsigned short int side;
};

std::unique_ptr<MeshBase>
LowerDBlockFromSidesetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Generate a new block id if one isn't supplied.
  SubdomainID new_block_id = isParamValid("new_block_id")
                                 ? getParam<SubdomainID>("new_block_id")
                                 : MooseMeshUtils::getNextFreeSubdomainID(*mesh);

  // Make sure our boundary info and parallel counts are setup
  if (!mesh->is_prepared())
  {
    const bool allow_remote_element_removal = mesh->allow_remote_element_removal();
    // We want all of our boundary elements available, so avoid removing them if they haven't
    // already been so
    mesh->allow_remote_element_removal(false);
    mesh->prepare_for_use();
    mesh->allow_remote_element_removal(allow_remote_element_removal);
  }

  auto sideset_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _sideset_names, true);
  std::set<boundary_id_type> sidesets(sideset_ids.begin(), sideset_ids.end());

  auto side_list = mesh->get_boundary_info().build_side_list();
  if (!mesh->is_serial() && mesh->comm().size() > 1)
  {
    std::vector<Elem *> elements_to_send;
    unsigned short i_need_boundary_elems = 0;
    for (const auto & [elem_id, side, bc_id] : side_list)
    {
      libmesh_ignore(side);
      if (sidesets.count(bc_id))
      {
        // Whether we have this boundary information through our locally owned element or a ghosted
        // element, we'll need the boundary elements for parallel consistent addition
        i_need_boundary_elems = 1;
        auto * elem = mesh->elem_ptr(elem_id);
        if (elem->processor_id() == mesh->processor_id())
          elements_to_send.push_back(elem);
      }
    }

    std::set<const Elem *, CompareElemIdsByLevel> connected_elements(elements_to_send.begin(),
                                                                     elements_to_send.end());
    std::set<const Node *> connected_nodes;
    reconnect_nodes(connected_elements, connected_nodes);
    std::set<dof_id_type> connected_node_ids;
    for (auto * nd : connected_nodes)
      connected_node_ids.insert(nd->id());

    std::vector<unsigned short> need_boundary_elems(mesh->comm().size());
    mesh->comm().allgather(i_need_boundary_elems, need_boundary_elems);
    std::unordered_map<processor_id_type, decltype(elements_to_send)> push_element_data;
    std::unordered_map<processor_id_type, decltype(connected_nodes)> push_node_data;

    for (const auto pid : index_range(mesh->comm()))
      // Don't need to send to self
      if (pid != mesh->processor_id() && need_boundary_elems[pid])
      {
        if (elements_to_send.size())
          push_element_data[pid] = elements_to_send;
        if (connected_nodes.size())
          push_node_data[pid] = connected_nodes;
      }

    auto node_action_functor = [](processor_id_type, const auto &)
    {
      // Node packing specialization already has unpacked node into mesh, so nothing to do
    };
    Parallel::push_parallel_packed_range(
        mesh->comm(), push_node_data, mesh.get(), node_action_functor);
    auto elem_action_functor = [](processor_id_type, const auto &)
    {
      // Elem packing specialization already has unpacked elem into mesh, so nothing to do
    };
    TIMPI::push_parallel_packed_range(
        mesh->comm(), push_element_data, mesh.get(), elem_action_functor);

    // now that we've gathered everything, we need to rebuild the side list
    side_list = mesh->get_boundary_info().build_side_list();
  }

  std::vector<std::pair<dof_id_type, ElemSideDouble>> element_sides_on_boundary;
  dof_id_type counter = 0;
  for (const auto & triple : side_list)
    if (sidesets.count(std::get<2>(triple)))
    {
      if (auto elem = mesh->query_elem_ptr(std::get<0>(triple)))
        element_sides_on_boundary.push_back(
            std::make_pair(counter, ElemSideDouble(elem, std::get<1>(triple))));
      ++counter;
    }

  dof_id_type max_elem_id = mesh->max_elem_id();
  unique_id_type max_unique_id = mesh->parallel_max_unique_id();

  // Making an important assumption that at least our boundary elements are the same on all
  // processes even in distributed mesh mode (this is reliant on the correct ghosting functors
  // existing on the mesh)
  for (auto & [i, elem_side] : element_sides_on_boundary)
  {
    Elem * elem = elem_side.elem;

    const auto side = elem_side.side;

    // Build a non-proxy element from this side.
    std::unique_ptr<Elem> side_elem(elem->build_side_ptr(side, /*proxy=*/false));

    // The side will be added with the same processor id as the parent.
    side_elem->processor_id() = elem->processor_id();

    // Add subdomain ID
    side_elem->subdomain_id() = new_block_id;

    // Also assign the side's interior parent, so it is always
    // easy to figure out the Elem we came from.
    side_elem->set_interior_parent(elem);

    // Add id
    side_elem->set_id(max_elem_id + i);
    side_elem->set_unique_id(max_unique_id + i);

    // Finally, add the lower-dimensional element to the Mesh.
    mesh->add_elem(side_elem.release());
  };

  // Assign block name, if provided
  if (isParamValid("new_block_name"))
    mesh->subdomain_name(new_block_id) = getParam<SubdomainName>("new_block_name");

  const bool skip_partitioning_old = mesh->skip_partitioning();
  mesh->skip_partitioning(true);
  mesh->prepare_for_use();
  mesh->skip_partitioning(skip_partitioning_old);

  return mesh;
}
