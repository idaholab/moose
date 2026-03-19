//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppGeneralFieldKDTreeTransferBase.h"
#include "KDTree.h"
#include "SolutionInvalidInterface.h"

/**
 * Performs a geometric interpolation based on the values at the nearest nodes to a target location
 * in the origin mesh.
 */
class MultiAppGeneralFieldNearestLocationTransfer : public MultiAppGeneralFieldKDTreeTransferBase

{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldNearestLocationTransfer(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void
  evaluateInterpValues(const unsigned int /*var_index*/,
                       const std::vector<std::pair<Point, unsigned int>> & incoming_points,
                       std::vector<std::pair<Real, Real>> & outgoing_vals) override;

private:
  void buildKDTrees(const unsigned int var_index) override;

  /*
   * Evaluate interpolation values for incoming points
   * @param incoming_points all the points at which we need values
   * @param outgoing_vals vector containing the values and distances from point to nearest node
   */
  void evaluateInterpValuesNearestNode(
      const std::vector<std::pair<Point, unsigned int>> & incoming_points,
      std::vector<std::pair<Real, Real>> & outgoing_vals);

  /// Whether the source of the values is at nodes (true) or centroids (false) for each variable
  std::vector<bool> _source_is_nodes;

  /// Whether we can just use the local zero-indexed dof to get the value from the solution
  std::vector<bool> _use_zero_dof_for_value;
};
