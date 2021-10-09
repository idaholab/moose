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

  /// List of boundarie(s) to refine
  const std::vector<BoundaryName> _boundaries;

  /// The amount of times to refine each boundary, corresponding to their index in 'boundaries'
  const std::vector<int> _refinement;

  /// Toggles whether neighboring level one elements should be refined or not. Defaults to true.
  const bool _enable_neighbor_refinement;

  /// Side(s) of the boundary/boundaries to be refined. Can be either "primary"(just the boundary elements), "secondary"(just the neighboring elements), or "both."
  const MultiMooseEnum _boundary_side;

  /**
   * The actual function refining the boundaries. This is done recursively in order
   * to minimize the number of refinement iterations to as little as possible.
   * @param boundary_ids Vector of boundary_ids to refine
   * @param mesh The mesh to refine
   * @param refinement Vector describing how many times to refine each boundary,
   * corresponding to their placement in boundary_ids
   * @param max Max value of all the refinement levels in the above vector
   * @param ref_step Step counter for the recursive function, defaults to 0 for initial call.
   * @return Unique pointer to a refined MeshBase
   */
  std::unique_ptr<MeshBase> recursive_refine(const std::vector<boundary_id_type> boundary_ids,
                                             std::unique_ptr<MeshBase> & mesh,
                                             const std::vector<int> refinement,
                                             const int max,
                                             int ref_step = 0);
};
