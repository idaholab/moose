//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockDeletionGenerator.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", BlockDeletionGenerator);

InputParameters
BlockDeletionGenerator::validParams()
{
  InputParameters params = ElementDeletionGeneratorBase::validParams();

  params.addClassDescription("Mesh generator which removes elements from the specified subdomains");
  params.addParam<std::vector<SubdomainName>>("block", "The list of blocks to be deleted");
  params.addDeprecatedParam<SubdomainID>(
      "block_id", "The block to be deleted.", "Use block instead");

  return params;
}

BlockDeletionGenerator::BlockDeletionGenerator(const InputParameters & parameters)
  : ElementDeletionGeneratorBase(parameters)
{
  // Handle deprecated parameter
  if (isParamValid("block_id"))
    _block_ids.push_back(getParam<SubdomainID>("block_id"));
  if (!isParamValid("block_id") && !isParamValid("block"))
    mooseError("Must provide the blocks to be deleted in the 'block' parameter");
  if (isParamValid("block_id") && isParamValid("block"))
    paramError("block_id", "Cannot use with the parameter 'block'. Please use just 'block'.");
}

std::unique_ptr<MeshBase>
BlockDeletionGenerator::generate()
{
  if (isParamValid("block"))
    // Get the list of block ids from the block names
    _block_ids =
        MooseMeshUtils::getSubdomainIDs(*_input, getParam<std::vector<SubdomainName>>("block"));

  // Check that the block ids/names exist in the mesh
  for (std::size_t i = 0; i < _block_ids.size(); ++i)
    if (!MooseMeshUtils::hasSubdomainID(*_input, _block_ids[i]))
    {
      if (isParamValid("block"))
        paramError("block",
                   "The block '",
                   getParam<std::vector<SubdomainName>>("block")[i],
                   "' was not found within the mesh");
      else
        paramError("block_id",
                   "The block '",
                   getParam<SubdomainID>("block_id"),
                   "' was not found within the mesh");
    }

  return ElementDeletionGeneratorBase::generate();
}

bool
BlockDeletionGenerator::shouldDelete(const Elem * elem)
{
  return std::find(_block_ids.begin(), _block_ids.end(), elem->subdomain_id()) != _block_ids.end();
}
