//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * MeshGenerator for refining one or more blocks
 */
class RefineBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  RefineBlockGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh to refine
  std::unique_ptr<MeshBase> & _input;

  /// List of block(s) to refine
  const std::vector<SubdomainName> _block;

  /// The amount of times to refine each block, corresponding to their index in 'block'
  const std::vector<unsigned int> _refinement;

  /// Toggles whether neighboring level one elements should be refined or not. Defaults to true.
  const bool _enable_neighbor_refinement;

  /**
   * The actual function refining the blocks. This is done recursively in order
   * to minimize the number of refinement iterations to as little as possible.
   * @param block_ids Vector of block_ids to refine
   * @param mesh The mesh to refine
   * @param refinement Vector describing how many times to refine each block, corresponding to
   * block_ids
   * @param max Max value of refinement param vector
   * @param ref_step Step counter for the recursive function, defaults to 0 for initial call.
   * @return Unique pointer to a refined MeshBase
   */
  virtual std::unique_ptr<MeshBase> recursive_refine(const std::vector<subdomain_id_type> block_ids,
                                                     std::unique_ptr<MeshBase> & mesh,
                                                     const std::vector<unsigned int> refinement,
                                                     const unsigned int max,
                                                     unsigned int ref_step = 0);
};
