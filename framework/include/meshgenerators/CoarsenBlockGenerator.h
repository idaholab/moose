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
 * MeshGenerator for coarsening one or more blocks
 */
class CoarsenBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CoarsenBlockGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh to coarsen
  std::unique_ptr<MeshBase> & _input;

  /// List of block(s) to coarsen
  const std::vector<SubdomainName> _block;

  /// The amount of times to coarsen each block, corresponding to their index in 'block'
  const std::vector<unsigned int> _coarsening;

  /// The ID of the element to start the coarsening from
  const unsigned int _start_elem_id;

  /**
   * The actual function coarsening the blocks. This is done recursively in order
   * to minimize the number of coarsening iterations to as little as possible.
   * @param block_ids Vector of block_ids to coarsen
   * @param mesh The mesh to coarsen
   * @param coarsening Vector describing how many times to coarsen each block, corresponding to
   * block_ids
   * @param max Max value of coarsening param vector
   * @param coarse_step Step counter for the recursive function
   * @return Unique pointer to a coarsened MeshBase
   */
  virtual std::unique_ptr<MeshBase>
  recursive_coarsen(const std::vector<subdomain_id_type> & block_ids,
                    std::unique_ptr<MeshBase> & mesh,
                    const std::vector<unsigned int> & coarsening,
                    const unsigned int max,
                    unsigned int coarse_step);
};
