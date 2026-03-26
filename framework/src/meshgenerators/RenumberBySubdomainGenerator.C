//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenumberBySubdomainGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

registerMooseObject("MooseApp", RenumberBySubdomainGenerator);

InputParameters
RenumberBySubdomainGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<SubdomainName>>(
      "blocks_to_renumber",
      "Elements and nodes within these blocks will be renumbered. If none are specified, all "
      "subdomains are affected by the renumbering");

  params.addClassDescription("Changes the element and node IDs so that elements and nodes are "
                             "contiguous within a subdomain. Note that DoF ordering may be "
                             "affected as well, and that the mesh renumbering will be turned off.");

  return params;
}

RenumberBySubdomainGenerator::RenumberBySubdomainGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
RenumberBySubdomainGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Not impossible to do.
  if (!mesh->is_serial())
    mooseError("Not implemented for non-serialized distributed meshes");

  // Orphaned nodes would cause problems on renumbering, we are looping on the nodes attached
  // to elements
  mesh->remove_orphaned_nodes();
  mesh->prepare_for_use();

  // Get the blocks provided by the user
  std::optional<std::vector<SubdomainName>> blocks =
      isParamValid("blocks_to_renumber")
          ? getParam<std::vector<SubdomainName>>("blocks_to_renumber")
          : std::vector<SubdomainName>();
  std::vector<SubdomainID> block_ids(blocks->size());
  std::stringstream missing_block;

  for (const auto i : index_range(block_ids))
  {
    const SubdomainName & name = blocks.value()[i];

    // Convert the SubdomainName to an id and store
    const auto id = MooseMeshUtils::getSubdomainID(name, *mesh);
    block_ids[i] = id;

    // Block does not exist - store for a future error
    if (id == Moose::INVALID_BLOCK_ID)
      missing_block << name << " ";
  }
  if (missing_block.str().size())
    paramError("blocks_to_renumber",
               "The following blocks were requested to be renumbered, but do not exist: ",
               missing_block.str());

  // User did not specify blocks, just get them all
  if (blocks->empty())
  {
    std::set<subdomain_id_type> block_ids_set;
    mesh->subdomain_ids(block_ids_set);
    block_ids.reserve(block_ids_set.size());
    block_ids.assign(block_ids_set.begin(), block_ids_set.end());

    // Does not have useful data
    blocks.reset();
  }

  // Renumber all elements with an ID we can recognize (so we can tell an already renumbered elem)
  const auto max_elem_id = mesh->max_elem_id();
  const auto max_node_id = mesh->max_node_id();
  std::unordered_map<dof_id_type, dof_id_type> new_elem_ids;
  new_elem_ids.reserve(max_elem_id);
  std::unordered_set<dof_id_type> renumbered_nodes;
  renumbered_nodes.reserve(max_node_id);

  // Renumber block IDs one at a time
  // We have to move them out of range, then back to range
  dof_id_type elem_count = 0;
  dof_id_type node_count = 0;
  for (const auto i : index_range(block_ids))
  {
    for (auto elem : mesh->active_subdomain_elements_ptr_range(block_ids[i]))
    {
      // If we didn't specify all the subdomains, we might have to skip elem ids that are still
      // taken
      while (mesh->query_elem_ptr(elem_count))
      {
        elem_count++;
      }
      // We can't mess with the range while looping in them
      new_elem_ids[elem->id()] = elem_count++;

      for (auto & node : elem->node_ref_range())
        // prevent re-renumbering
        if (!renumbered_nodes.count(node.id()))
        {
          // If we didn't specify all the subdomains, we might have to skip node ids that are still
          // taken
          while (mesh->query_node_ptr(node_count))
          {
            node_count++;
          }
          renumbered_nodes.insert(node.id());
          mesh->renumber_node(node.id(), node_count++);
        }
    }
  }

  // Now change the IDs
  for (const auto [key, value] : new_elem_ids)
    mesh->renumber_elem(key, value);

  // Update the max ids
  mesh->contract();

  // Try to disallow renumbering from now on since we just renumbered
  mesh->allow_renumbering(false);
  // No gain if exodus just renumbers this. Better to tell the user
  if (mesh->n_nodes() != mesh->max_node_id() || mesh->n_elem() != mesh->max_elem_id())
    mooseWarning("Mesh is not contiguously numbered after renumbering. The numbering may be erased "
                 "by outputs that require contiguous numbering such as Exodus.\nNumber of nodes: " +
                 std::to_string(mesh->n_nodes()) +
                 "\nMax node ID: " + std::to_string(mesh->max_node_id() - 1) +
                 "\nNumber of elements: " + std::to_string(mesh->n_elem()) +
                 "\nMax elem ID: " + std::to_string(mesh->max_elem_id() - 1));

  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
