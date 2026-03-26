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
  for (const auto i : index_range(block_ids))
    for (auto elem : mesh->active_subdomain_elements_ptr_range(block_ids[i]))
    {
      mesh->renumber_elem(elem->id(), max_elem_id + 1 + elem->id());

      // Renumber all nodes with an ID we can recognize (so we can tell an already renumbered node)
      for (auto & node : elem->node_ref_range())
        if (node.id() <= max_node_id)
          mesh->renumber_node(node.id(), max_node_id + 1 + node.id());
    }

  // Renumber block IDs one at a time
  // We have to move them out of range, then back to range
  dof_id_type elem_count = 0;
  dof_id_type node_count = 0;
  for (const auto i : index_range(block_ids))
  {
    for (auto elem : mesh->active_subdomain_elements_ptr_range(block_ids[i]))
    {
      // should always be true
      if (elem->id() > max_elem_id)
        mesh->renumber_elem(elem->id(), elem_count++);

      for (auto & node : elem->node_ref_range())
        // prevent re-renumbering
        if (node.id() > max_node_id)
          mesh->renumber_node(node.id(), node_count++);
    }
  }
  // Try to disallow renumbering from now on since we just renumbered
  mesh->allow_renumbering(false);
  // No gain if exodus just renumbers this. Better to tell the user
  if (mesh->n_nodes() != mesh->max_node_id() || mesh->n_elem() != mesh->max_elem_id())
    mooseWarning("Mesh is not contiguously numbered after renumbering. The numbering may be erased "
                 "by outputs that require contiguous numbering such as Exodus");

  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
