//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenameBlock.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<RenameBlock>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addParam<std::vector<SubdomainID>>(
      "old_block_id",
      "Elements with this block number will be given the new_block_number or "
      "new_block_name.  You must supply either old_block_id or old_block_name.  "
      "You may supply a vector of old_block_id, in which case the new_block "
      "information must also be a vector.");
  params.addParam<std::vector<SubdomainName>>(
      "old_block_name",
      "Elements with this block name will be given the new_block_number or "
      "new_block_name.  You must supply either old_block_id or old_block_name.  "
      "You may supply a vector of old_block_name, in which case the new_block "
      "information must also be a vector.");
  params.addParam<std::vector<SubdomainID>>(
      "new_block_id",
      "Elements with the old block number (or name) will be given this block "
      "number.  If the old blocks are named, their names will be passed onto the "
      "newly numbered blocks.");
  params.addParam<std::vector<SubdomainName>>(
      "new_block_name",
      "Elements with the old block number (or name) will be given this block "
      "name.  No change of block ID is performed, unless multiple old blocks are "
      "given the same name, in which case they are all given the first old block "
      "number.");
  params.addClassDescription(
      "RenameBlock re-numbers or re-names an old_block_id or old_block_name "
      "with a new_block_id or new_block_name.  If using RenameBlock to "
      "merge blocks (by giving them the same name, for instance) it is "
      "advisable to specify all your blocks in old_blocks to avoid inconsistencies");
  return params;
}

RenameBlock::RenameBlock(const InputParameters & parameters) : MeshModifier(parameters)
{
  // error checking.  Must have exactly one of old_block_id or old_block_name
  if (isParamValid("old_block_id") && isParamValid("old_block_name"))
    mooseError("RenameBlock: You must supply exactly one of old_block_id or old_block_name\n");
  else if (!isParamValid("old_block_id") && !isParamValid("old_block_name"))
    mooseError("RenameBlock: You must supply exactly one of old_block_id or old_block_name\n");

  // error checking.  Must have exactly one of new_block_id or new_block_name
  // In principal we could have both (the old block would then be given a new ID and a new name)
  // but i feel that could lead to confusion for the user.  If the user wants to do that they
  // should use two of these RenameBlock MeshModifiers.
  if (isParamValid("new_block_id") && isParamValid("new_block_name"))
    mooseError("RenameBlock: You must supply exactly one of new_block_id or new_block_name\n");
  else if (!isParamValid("new_block_id") && !isParamValid("new_block_name"))
    mooseError("RenameBlock: You must supply exactly one of new_block_id or new_block_name\n");
}

void
RenameBlock::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling RenameBlock::modify()");

  // grab the user input.  Can't do all this in the constructor as some things may not
  // have been put into the mesh yet, eg old_block_name might have been inserted by
  // another MeshModifier
  if (isParamValid("old_block_id"))
  {
    // user must have supplied old_block_id
    _old_block_id = getParam<std::vector<SubdomainID>>("old_block_id");
    _old_block_name.reserve(_old_block_id.size());
    for (const auto & block_id : _old_block_id)
      _old_block_name.emplace_back(_mesh_ptr->getMesh().subdomain_name(block_id));
  }
  else
  {
    // user must have supplied old_block_name
    _old_block_name = getParam<std::vector<SubdomainName>>("old_block_name");
    _old_block_id = _mesh_ptr->getSubdomainIDs(_old_block_name);
  }

  // construct new_block_id and new_block_name
  if (isParamValid("new_block_id"))
  {
    _new_block_id = getParam<std::vector<SubdomainID>>("new_block_id");
    if (_new_block_id.size() != _old_block_id.size())
      mooseError("RenameBlock: The vector of old_block information must have the same length as "
                 "the vector of new_block information\n");

    // construct the _new_block_name
    _new_block_name.reserve(_new_block_id.size());
    for (const auto & block_id : _new_block_id)
      _new_block_name.emplace_back(newBlockName(block_id));
  }
  else // must have supplied new_block_name
  {
    _new_block_name = getParam<std::vector<SubdomainName>>("new_block_name");
    if (_new_block_name.size() != _old_block_id.size())
      mooseError("RenameBlock: The vector of old_block information must have the same length as "
                 "the vector of new_block information\n");

    _new_block_id.reserve(_new_block_name.size());
    for (const auto & block_name : _new_block_name)
      _new_block_id.emplace_back(newBlockID(block_name));
  }

  for (const auto & elem : _mesh_ptr->getMesh().active_element_ptr_range())
    for (unsigned i = 0; i < _old_block_id.size(); ++i)
      if (elem->subdomain_id() == _old_block_id[i])
        elem->subdomain_id() = _new_block_id[i];
  for (unsigned i = 0; i < _old_block_id.size(); ++i)
    _mesh_ptr->getMesh().subdomain_name(_old_block_id[i]) = _new_block_name[i];
}

const SubdomainName
RenameBlock::newBlockName(const SubdomainID & new_block_id)
{
  for (unsigned i = 0; i < _new_block_id.size(); ++i)
    if (_new_block_id[i] == new_block_id)
      return _old_block_name[i];
  mooseError("RenameBlock: error in code");
}

const SubdomainID
RenameBlock::newBlockID(const SubdomainName & new_block_name)
{
  for (unsigned i = 0; i < _new_block_name.size(); ++i)
    if (_new_block_name[i] == new_block_name)
      return _old_block_id[i];
  mooseError("RenameBlock: error in code");
}
