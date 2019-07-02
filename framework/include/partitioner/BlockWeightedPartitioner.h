//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseEnum.h"
#include "PetscExternalPartitioner.h"

class BlockWeightedPartitioner;
class MooseMesh;

template <>
InputParameters validParams<BlockWeightedPartitioner>();

/**
 * Partition a mesh by weighting blocks. The motivation is that differenct
 * blocks may have different physics. The work load per element is different
 * block-by-block. This partitioner allows users to assign high weights for
 * heavy blocks and low weights for other light blocks.
 */
class BlockWeightedPartitioner : public PetscExternalPartitioner
{
public:
  BlockWeightedPartitioner(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm) override;

private:

  /// Vector the block names supplied by the user via the input file
  std::vector<SubdomainName> _blocks;
  /// Block weights
  std::vector<dof_id_type>   _weights;
  /// A map from subdomain to weight
  std::unordered_map<SubdomainID, dof_id_type> _blocks_to_weights;
  /// Moose mesh
  MooseMesh & _mesh;
};
