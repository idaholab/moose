//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockWeightedPartitioner.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", BlockWeightedPartitioner);

template <>
InputParameters
validParams<BlockWeightedPartitioner>()
{
  InputParameters params = validParams<PetscExternalPartitioner>();

  params.addRequiredParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) that this object will be applied");

  params.addRequiredParam<std::vector<dof_id_type>>(
      "weight", "The list of weights (integer) that specify how heavy each block is");

  params.set<bool>("apply_element_weight") = true;

  params.addClassDescription(
      "Partition mesh by weighting blocks");

  return params;
}

BlockWeightedPartitioner::BlockWeightedPartitioner(const InputParameters & params)
  : PetscExternalPartitioner(params),
  _blocks(getParam<std::vector<SubdomainName>>("block")),
  _weights(getParam<std::vector<dof_id_type>>("weight")),
  _mesh(*getParam<MooseMesh *>("mesh"))
{
  if (_blocks.size() != _weights.size())
    paramError("block", "Number of weights ",_weights.size(), " does not match with the number of blocks ", _blocks.size());

  // Get the IDs from the supplied names
  auto block_ids = _mesh.getSubdomainIDs(_blocks);

  if (block_ids.size() != _blocks.size())
    mooseError("Number of block ids ", block_ids.size(), " does not match with the number of blocks ", _blocks.size());

  _blocks_to_weights.reserve(_weights.size());

  for (MooseIndex(block_ids.size()) i = 0; i<block_ids.size(); i++)
  {
    _blocks_to_weights[block_ids[i]] = _weights[i];
  }
}

std::unique_ptr<Partitioner>
BlockWeightedPartitioner::clone() const
{
  return libmesh_make_unique<BlockWeightedPartitioner>(_pars);
}

dof_id_type
BlockWeightedPartitioner::computeElementWeight(Elem & elem)
{
  auto blockid_to_weight = _blocks_to_weights.find(elem.subdomain_id());

  if (blockid_to_weight == _blocks_to_weights.end())
    mooseError("Can not find a weight for block id ",elem.subdomain_id());

  return blockid_to_weight->second;
}
