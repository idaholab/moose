//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockWeightedPartitioner.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", BlockWeightedPartitioner);

InputParameters
BlockWeightedPartitioner::validParams()
{
  InputParameters params = PetscExternalPartitioner::validParams();

  params.addRequiredParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) that this object will be applied");

  params.addRequiredParam<std::vector<dof_id_type>>(
      "weight", "The list of weights (integer) that specify how heavy each block is");

  params.set<bool>("apply_element_weight") = true;

  params.addClassDescription("Partition mesh by weighting blocks");

  return params;
}

BlockWeightedPartitioner::BlockWeightedPartitioner(const InputParameters & params)
  : PetscExternalPartitioner(params),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _weights(getParam<std::vector<dof_id_type>>("weight"))
{
  if (_blocks.size() != _weights.size())
    paramError("block",
               "Number of weights ",
               _weights.size(),
               " does not match with the number of blocks ",
               _blocks.size());
}

std::unique_ptr<Partitioner>
BlockWeightedPartitioner::clone() const
{
  return std::make_unique<BlockWeightedPartitioner>(_pars);
}

void
BlockWeightedPartitioner::initialize(MeshBase & mesh)
{
  // Get the IDs from the supplied names
  const auto block_ids = MooseMeshUtils::getSubdomainIDs(mesh, _blocks);

  // Make sure all of the blocks exist
  std::set<subdomain_id_type> mesh_block_ids;
  mesh.subdomain_ids(mesh_block_ids);
  for (const auto block_id : block_ids)
    if (!mesh_block_ids.count(block_id))
      paramError("block", "The block ", block_id, " was not found on the mesh");

  // Setup the block -> weight map for use in computeElementWeight
  _blocks_to_weights.reserve(_weights.size());
  for (MooseIndex(block_ids.size()) i = 0; i < block_ids.size(); i++)
    _blocks_to_weights[block_ids[i]] = _weights[i];
}

dof_id_type
BlockWeightedPartitioner::computeElementWeight(Elem & elem)
{
  mooseAssert(_blocks_to_weights.count(elem.subdomain_id()), "Missing weight for block");
  return _blocks_to_weights[elem.subdomain_id()];
}
