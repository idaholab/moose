//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenameBlockGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", RenameBlockGenerator);

InputParameters
RenameBlockGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  params.addDeprecatedParam<std::vector<SubdomainID>>(
      "old_block_id",
      "Elements with this block number will be given the new_block_number or "
      "new_block_name.  You must supply either old_block_id or old_block_name.  "
      "You may supply a vector of old_block_id, in which case the new_block "
      "information must also be a vector.",
      "Use 'old_block' instead of 'old_block_id'.");
  params.addDeprecatedParam<std::vector<SubdomainName>>(
      "old_block_name",
      "Elements with this block name will be given the new_block_number or "
      "new_block_name.  You must supply either old_block_id or old_block_name.  "
      "You may supply a vector of old_block_name, in which case the new_block "
      "information must also be a vector.",
      "Use 'old_block' instead of 'old_block_name'.");
  params.addDeprecatedParam<std::vector<SubdomainID>>(
      "new_block_id",
      "Elements with the old block number (or name) will be given this block "
      "number.  If the old blocks are named, their names will be passed onto the "
      "newly numbered blocks.",
      "Use 'new_block' instead of 'new_block_id'.");
  params.addDeprecatedParam<std::vector<SubdomainName>>(
      "new_block_name",
      "Elements with the old block number (or name) will be given this block "
      "name.  No change of block ID is performed, unless multiple old blocks are "
      "given the same name, in which case they are all given the first old block "
      "number.",
      "Use 'new_block' instead of 'new_block_name'.");

  params.addParam<std::vector<SubdomainName>>(
      "old_block",
      "Elements with these block ID(s)/name(s) will be given the new block information specified "
      "in 'new_block'");
  params.addParam<std::vector<SubdomainName>>(
      "new_block",
      "The new block ID(s)/name(s) to be given by the elements defined in 'old_block'.");

  params.addClassDescription("Changes the block IDs and/or block names for a given set of blocks "
                             "defined by either block ID or block name. The changes are "
                             "independent of ordering. The merging of blocks is supported.");

  return params;
}

RenameBlockGenerator::RenameBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
  if (isParamValid("old_block_id") && isParamValid("old_block_name"))
    paramError("old_block_id",
               "Cannot use in combination with 'old_block_name'. Please use 'old_block' "
               "instead; 'old_block_id' and 'old_block_name' are deprecated.");
  if (isParamValid("new_block_id") && isParamValid("new_block_name"))
    paramError("new_block_id",
               "Cannot use in combination with 'new_block_name'. Please use 'new_block' "
               "instead; 'new_block_id' and 'new_block_name' are deprecated.");

  if (isParamValid("old_block"))
  {
    if (isParamValid("old_block_id"))
      paramError("old_block_id",
                 "Cannot use with 'old_block'. Use only 'old_block'; 'old_block_id' is "
                 "deprecated.");
    if (isParamValid("old_block_name"))
      paramError("old_block_name",
                 "Cannot use with 'old_block'. Use only 'old_block'; 'old_block_name' is "
                 "deprecated.");
    _old_block = getParam<std::vector<SubdomainName>>("old_block");
    _old_block_param_name = "old_block";
  }
  else if (isParamValid("old_block_id"))
  {
    for (const auto id : getParam<std::vector<SubdomainID>>("old_block_id"))
      _old_block.push_back(std::to_string(id));
    _old_block_param_name = "old_block_id";
  }
  else
  {
    _old_block = getParam<std::vector<SubdomainName>>("old_block_name");
    _old_block_param_name = "old_block_name";
  }

  std::string new_block_param_name;
  if (isParamValid("new_block"))
  {
    if (isParamValid("new_block_id"))
      paramError("new_block_id",
                 "Cannot use with 'new_block'. Use only 'new_block'; 'new_block_id' is "
                 "deprecated.");
    if (isParamValid("new_block_name"))
      paramError("new_block_name",
                 "Cannot use with 'new_block'. Use only 'new_block'; 'new_block_name' is "
                 "deprecated.");
    _new_block = getParam<std::vector<SubdomainName>>("new_block");
    new_block_param_name = "new_block";
  }
  else if (isParamValid("new_block_id"))
  {
    for (const auto id : getParam<std::vector<SubdomainID>>("new_block_id"))
      _new_block.push_back(std::to_string(id));
    new_block_param_name = "new_block_id";
  }
  else
  {
    _new_block = getParam<std::vector<SubdomainName>>("new_block_name");
    new_block_param_name = "new_block_name";
  }

  if (_old_block.size() != _new_block.size())
    paramError(new_block_param_name, "Must be the same length as '", _old_block_param_name, "'");
}

std::unique_ptr<MeshBase>
RenameBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // MeshBase::subdomain_name will insert, so we need a const ref
  const MeshBase & const_mesh = *mesh;

  // Get the subdomains in the mesh (this is global)
  std::set<subdomain_id_type> block_ids;
  mesh->subdomain_ids(block_ids);

  // Helper for getting an unused block ID, and keeping track of it
  // so that we can generate more later
  auto get_unused_block_id = [this, &block_ids, &const_mesh]()
  {
    for (const auto id : make_range(Moose::INVALID_BLOCK_ID))
      if (!block_ids.count(id) && !const_mesh.get_subdomain_name_map().count(id))
      {
        block_ids.insert(id);
        return id;
      }

    mooseError("Failed to find an unused ID!");
  };

  const auto num_blocks = _old_block.size();

  // Helper for checking whether or not a SubdomainName (which could be an ID or a name)
  // is really input as an ID
  const auto is_subdomain_id = [](const SubdomainName & subdomain_name)
  { return MooseUtils::isDigits(subdomain_name); };

  // Get the old block IDs and make sure they exist
  std::vector<SubdomainID> old_block_ids(num_blocks, Moose::INVALID_BLOCK_ID);
  std::vector<SubdomainName> old_block_names(num_blocks);
  std::stringstream missing_block;
  for (const auto i : make_range(num_blocks))
  {
    const SubdomainName & name = _old_block[i];

    // Convert the SubdomainName to an id and store
    const auto id = MooseMeshUtils::getSubdomainID(name, *mesh);
    old_block_ids[i] = id;

    // Block does not exist - store for a future error
    if (!block_ids.count(id))
      missing_block << name << " ";

    // Keep track of the block names
    // If this SubdomainName is an ID, try to see if it has a name set
    if (is_subdomain_id(name))
      old_block_names[i] = const_mesh.subdomain_name(id);
    // If this SubdomainName is a name, use said name
    else
      old_block_names[i] = name;
  }
  if (missing_block.str().size())
    paramError(_old_block_param_name,
               "The following blocks were requested to be renamed, but do not exist: ",
               missing_block.str());

  // Get the block IDs that we're moving to
  std::vector<SubdomainID> new_block_ids(num_blocks, Moose::INVALID_BLOCK_ID);
  std::map<SubdomainID, std::string> new_names;
  for (const auto i : make_range(num_blocks))
  {
    const SubdomainName & name = _new_block[i];

    // If the user input an ID, we have the ID
    if (is_subdomain_id(name))
    {
      const auto id = MooseMeshUtils::getSubdomainID(name, *mesh);
      new_block_ids[i] = id;

      // In the case that this is a new block ID, keep track of it so that we
      // don't reuse it if we have to create temporaries
      block_ids.insert(id);

      // Preserve the old block name if there was one
      if (old_block_names[i].size())
        new_names[id] = old_block_names[i];
    }
    // If the user input a name, we will use the ID that it is coming from for the
    // "new" name if the new name does not name a current block. If the name does
    // exist, we will merge with said block.
    else
    {
      bool name_already_exists = false;

      // If the target block already exists, merge into that one
      // Check both the old maps and the new map
      for (const auto map : {&const_mesh.get_subdomain_name_map(),
                             const_cast<const std::map<SubdomainID, std::string> *>(&new_names)})
        for (const auto & id_name_pair : *map)
          if (!name_already_exists && id_name_pair.second == name)
          {
            new_block_ids[i] = id_name_pair.first;
            new_names[id_name_pair.first] = name;
            name_already_exists = true;
          }

      // Target name doesn't exist, so use the source id/name
      if (!name_already_exists)
      {
        new_block_ids[i] = old_block_ids[i];
        new_names[new_block_ids[i]] = name;
      }
    }
  }

  // Create temporaries if needed; recall that this generator is independent
  // of input ordering and does _not_ merge subdomains.
  //
  // Take the example where we want to move 0 -> 1 and 1 -> 2. If we just
  // move them in order, we will actually end up with (0, 1) -> 2. This is
  // bad. In this case, we want to first make a temporary for 1 (call it 3).
  // We then do: 0 -> 3, 1 -> 2, 3 -> 1 in order to get the desired behavior.
  // We will accomplish this by creating temporaries as needed, modifying
  // the initial move to the temporaries as needed, and then moving the
  // temporaries back. temp_change_ids here are the (from -> to) pairs
  // that we will move at the end.
  auto temp_new_block_ids = new_block_ids;
  std::vector<std::pair<SubdomainID, SubdomainID>> temp_change_ids;
  // Loop through all new IDs
  for (const auto new_i : make_range(num_blocks))
  {
    // Look at all of the old IDs that will be moved after the move to the new ID.
    // If any of the old IDs after are IDs that we are moving to, create a temporary
    // and keep track of it so we can move it back at the end.
    for (const auto old_i : make_range(new_i + 1, num_blocks))
      if (new_block_ids[new_i] == old_block_ids[old_i])
      {
        const auto temp_id = get_unused_block_id();
        temp_change_ids.emplace_back(temp_id, new_block_ids[new_i]);
        temp_new_block_ids[new_i] = temp_id;
        break;
      }
  }

  // First pass through changing the block ids
  for (const auto i : make_range(num_blocks))
    MeshTools::Modification::change_subdomain_id(*mesh, old_block_ids[i], temp_new_block_ids[i]);
  // Pass through moving the temporaries to the actual blocks, if necessary
  for (const auto & pair : temp_change_ids)
    MeshTools::Modification::change_subdomain_id(*mesh, pair.first, pair.second);

  // First go through and remove all of the old names
  for (const auto i : make_range(num_blocks))
    if (mesh->get_subdomain_name_map().count(old_block_ids[i]))
      mesh->set_subdomain_name_map().erase(old_block_ids[i]);
  // With the old names removed, add the new names if there are any to add
  for (const auto & pair : new_names)
    mesh->subdomain_name(pair.first) = pair.second;

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
