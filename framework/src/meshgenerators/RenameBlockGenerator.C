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

#include "libmesh/elem.h"

registerMooseObject("MooseApp", RenameBlockGenerator);

template <>
InputParameters
validParams<RenameBlockGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<subdomain_id_type>>(
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
  params.addParam<std::vector<subdomain_id_type>>(
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

RenameBlockGenerator::RenameBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
  // error checking.  Must have exactly one of old_block_id or old_block_name
  if (isParamValid("old_block_id") && isParamValid("old_block_name"))
    mooseError(
        "RenameBlockGenerator: You must supply exactly one of old_block_id or old_block_name\n");
  else if (!isParamValid("old_block_id") && !isParamValid("old_block_name"))
    mooseError(
        "RenameBlockGenerator: You must supply exactly one of old_block_id or old_block_name\n");

  // error checking.  Must have exactly one of new_block_id or new_block_name
  // In principal we could have both (the old block would then be given a new ID and a new name)
  // but i feel that could lead to confusion for the user.  If the user wants to do that they
  // should use two of these RenameBlock MeshModifiers.
  if (isParamValid("new_block_id") && isParamValid("new_block_name"))
    mooseError(
        "RenameBlockGenerator: You must supply exactly one of new_block_id or new_block_name\n");
  else if (!isParamValid("new_block_id") && !isParamValid("new_block_name"))
    mooseError(
        "RenameBlockGenerator: You must supply exactly one of new_block_id or new_block_name\n");
}

std::unique_ptr<MeshBase>
RenameBlockGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // grab the user input.  Can't do all this in the constructor as some things may not
  // have been put into the mesh yet, eg old_block_name might have been inserted by
  // another MeshModifier
  if (isParamValid("old_block_id"))
  {
    // user must have supplied old_block_id
    _old_block_id = getParam<std::vector<subdomain_id_type>>("old_block_id");
    _old_block_name.reserve(_old_block_id.size());
    for (const auto & block_id : _old_block_id)
      _old_block_name.emplace_back(mesh->subdomain_name(block_id));
  }
  else
  {
    // user must have supplied old_block_name
    _old_block_name = getParam<std::vector<SubdomainName>>("old_block_name");
    _old_block_id.reserve(_old_block_name.size());
    for (const auto & block_name : _old_block_name)
      _old_block_id.emplace_back(mesh->get_id_by_name(block_name));
  }

  // construct new_block_id and new_block_name
  if (isParamValid("new_block_id"))
  {
    _new_block_id = getParam<std::vector<subdomain_id_type>>("new_block_id");
    if (_new_block_id.size() != _old_block_id.size())
      mooseError("RenameBlockGenerator: The vector of old_block information must have the same"
                 " length as the vector of new_block information\n");

    // construct the _new_block_name
    _new_block_name.reserve(_new_block_id.size());
    for (const auto & block_id : _new_block_id)
      _new_block_name.emplace_back(newBlockName(block_id));
  }
  else // must have supplied new_block_name
  {
    _new_block_name = getParam<std::vector<SubdomainName>>("new_block_name");
    if (_new_block_name.size() != _old_block_id.size())
      mooseError("RenameBlockGenerator: The vector of old_block information must have the same"
                 " length as the vector of new_block information\n");

    _new_block_id.reserve(_new_block_name.size());
    for (const auto & block_name : _new_block_name)
      _new_block_id.emplace_back(newBlockID(block_name));
  }

  for (const auto & elem : mesh->active_element_ptr_range())
    for (unsigned i = 0; i < _old_block_id.size(); ++i)
      if (elem->subdomain_id() == _old_block_id[i])
        elem->subdomain_id() = _new_block_id[i];
  for (unsigned i = 0; i < _old_block_id.size(); ++i)
    mesh->subdomain_name(_old_block_id[i]) = _new_block_name[i];

  return dynamic_pointer_cast<MeshBase>(mesh);
}

SubdomainName
RenameBlockGenerator::newBlockName(const subdomain_id_type & new_block_id)
{
  for (unsigned i = 0; i < _new_block_id.size(); ++i)
    if (_new_block_id[i] == new_block_id)
      return _old_block_name[i];
  mooseError("RenameBlock: error in code");
}

subdomain_id_type
RenameBlockGenerator::newBlockID(const SubdomainName & new_block_name)
{
  for (unsigned i = 0; i < _new_block_name.size(); ++i)
    if (_new_block_name[i] == new_block_name)
      return _old_block_id[i];
  mooseError("RenameBlock: error in code");
}
