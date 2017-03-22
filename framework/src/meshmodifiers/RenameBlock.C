/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
      "number.  You must supply either new_block_id or new_block_name.  You may "
      "supply a vector of new_block_id, in which case the old_block information "
      "must also be a vector.");
  params.addParam<std::vector<SubdomainName>>(
      "new_block_name",
      "Elements with the old block number (or name) will be given this block "
      "name.  You must supply either new_block_id or new_block_name.  You may "
      "supply a vector of new_block_id, in which case the old_block information "
      "must also be a vector.");
  params.addClassDescription("RenameBlock re-numbers or re-names an old_block_id or old_block_name "
                             "with a new_block_id or new_block_name");
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
    _old_block_id = getParam<std::vector<SubdomainID>>("old_block_id");
  else // must have supplied old_block_name
    _old_block_id =
        _mesh_ptr->getSubdomainIDs(getParam<std::vector<SubdomainName>>("old_block_name"));

  if (isParamValid("new_block_id"))
  {
    _new_block_id = getParam<std::vector<SubdomainID>>("new_block_id");
    if (_new_block_id.size() != _old_block_id.size())
      mooseError("RenameBlock: The vector of old_block information must have the same length as "
                 "the vector of new_block information\n");
    for (MeshBase::element_iterator el = _mesh_ptr->getMesh().active_elements_begin();
         el != _mesh_ptr->getMesh().active_elements_end();
         ++el)
      for (unsigned i = 0; i < _old_block_id.size(); ++i)
        if ((*el)->subdomain_id() == _old_block_id[i])
          (*el)->subdomain_id() = _new_block_id[i];
  }
  else // must have supplied new_block_name
  {
    _new_block_name = getParam<std::vector<SubdomainName>>("new_block_name");
    if (_new_block_name.size() != _old_block_id.size())
      mooseError("RenameBlock: The vector of old_block information must have the same length as "
                 "the vector of new_block information\n");
    for (unsigned i = 0; i < _old_block_id.size(); ++i)
      // libmesh appears to check that _old_block_id[i] isn't too big or too small
      _mesh_ptr->getMesh().subdomain_name(_old_block_id[i]) = _new_block_name[i];
  }
}
