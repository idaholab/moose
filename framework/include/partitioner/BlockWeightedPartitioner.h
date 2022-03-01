//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PetscExternalPartitioner.h"

/**
 * Partition a mesh by weighting blocks. The motivation is that differenct
 * blocks may have different physics. The work load per element is different
 * block-by-block. This partitioner allows users to assign high weights for
 * heavy blocks and low weights for other light blocks.
 */
class BlockWeightedPartitioner : public PetscExternalPartitioner
{
public:
  static InputParameters validParams();

  BlockWeightedPartitioner(const InputParameters & params);

  virtual std::unique_ptr<Partitioner> clone() const override;

  virtual dof_id_type computeElementWeight(Elem & elm) override;

  /**
   * Fills _blocks_to_weights before performing the partition
   */
  void initialize(MeshBase & mesh) override;

private:
  /// Vector the block names supplied by the user via the input file
  const std::vector<SubdomainName> & _blocks;
  // Block weights
  const std::vector<dof_id_type> & _weights;

  /// A map from subdomain to weight
  std::unordered_map<SubdomainID, dof_id_type> _blocks_to_weights;
};
