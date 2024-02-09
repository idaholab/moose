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

  /// The location on the mesh to start the coarsening from
  const Point _starting_point;

  /// Maximum volume ratio between a neighbor and an element to consider the neighbor as a
  /// candidate for coarsening after coarsening the element
  const Real _max_vol_ratio;

  /// Whether the mesh generator should be verbose to the console
  const bool _verbose;

  /// Whether to check the output mesh for non-conformality
  const bool _check_output_mesh_for_nonconformality;

  /**
   * The actual function coarsening the blocks.
   * @param block_ids Vector of block_ids to coarsen
   * @param mesh The mesh to coarsen
   * @param coarsening Vector describing how many times to coarsen each block, corresponding to
   * block_ids
   * @param max Max value of coarsening param vector / maximum block coarsening requested
   * @param coarse_step Step counter for the recursive function
   * @return Unique pointer to a coarsened MeshBase
   */
  virtual std::unique_ptr<MeshBase>
  recursiveCoarsen(const std::vector<subdomain_id_type> & block_ids,
                   std::unique_ptr<MeshBase> & mesh,
                   const std::vector<unsigned int> & coarsening,
                   const unsigned int max,
                   unsigned int coarse_step);
};
