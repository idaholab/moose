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
 * MeshGenerator for refining one or more sidesets
 */
class RefineSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  RefineSidesetGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh to refine
  std::unique_ptr<MeshBase> & _input;

  /// List of block(s) to refine
  const std::vector<BoundaryName> _boundaries;

  /// The amount of times to refine each block, corresponding to their index in 'block'
  const std::vector<int> _refinement;

  /// Toggles whether neighboring level one elements should be refined or not. Defaults to true.
  const bool _enable_neighbor_refinement;

  /// Side(s) of the boundary to be refined
  const std::vector<MooseEnum> _boundary_side;

  /// The actual function refining the blocks. This is done recursively in order to minimize the number of refinement iterations to as little as possible.
  virtual std::unique_ptr<MeshBase> recursive_refine(const std::vector<boundary_id_type> boundary_ids,
                                                     const std::vector<MooseEnum> boundary_side,
                                                     std::unique_ptr<MeshBase> & mesh,
                                                     const std::vector<int> refinement,
                                                     const int max,
                                                     int ref_step = 0);
};
