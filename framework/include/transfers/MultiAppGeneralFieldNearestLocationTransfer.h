//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppGeneralFieldTransfer.h"
#include "KDTree.h"
#include "SolutionInvalidInterface.h"

/**
 * Performs a geometric interpolation based on the values at the nearest nodes to a target location
 * in the origin mesh.
 */
class MultiAppGeneralFieldNearestLocationTransfer : public MultiAppGeneralFieldTransfer,
                                                    public SolutionInvalidInterface

{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldNearestLocationTransfer(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void prepareEvaluationOfInterpValues(const unsigned int var_index) override;

  virtual void
  evaluateInterpValues(const std::vector<std::pair<Point, unsigned int>> & incoming_points,
                       std::vector<std::pair<Real, Real>> & outgoing_vals) override;

  using MultiAppGeneralFieldTransfer::inBlocks;
  bool inBlocks(const std::set<SubdomainID> & blocks,
                const MooseMesh & mesh,
                const Elem * elem) const override;

private:
  bool usesMooseAppCoordTransform() const override { return true; }
  /*
   * Build KD-Trees for each local app
   * @param var_index the index of the variable being transferred
   * @details fills _local_kdtrees, _local_points and _local_values
   *          Indexing is: local apps (outer-indexing) OR positions (if using nearest_positions),
   *          local nodes (inner-indexing)
   */
  void buildKDTrees(const unsigned int var_index);

  /*
   * Evaluate interpolation values for incoming points
   * @param incoming_points all the points at which we need values
   * @param outgoing_vals vector containing the values and distances from point to nearest node
   */
  void evaluateInterpValuesNearestNode(
      const std::vector<std::pair<Point, unsigned int>> & incoming_points,
      std::vector<std::pair<Real, Real>> & outgoing_vals);

  /// Pre-compute the number of sources
  /// Number of KDTrees used to hold the locations and variable value data
  void computeNumSources();

  /// Get the index of the app in the loop over the trees and the apps contributing data to each tree
  unsigned int getAppIndex(unsigned int kdtree_index, unsigned int app_index) const;

  /// Number of applications which contributed nearest-locations to each KD-tree
  unsigned int getNumAppsPerTree() const;

  /// Number of divisions (nearest-positions or source mesh divisions) used when building KD-Trees
  unsigned int getNumDivisions() const;

  /// Transform a point towards the local frame
  Point getPointInLocalSourceFrame(unsigned int i_from, const Point & pt) const;

  /**
   * @brief Examine all spatial restrictions that could preclude this source from being
   *        a valid source for this point
   * @param pt point of interest
   * @param valid_mesh_div if using source mesh divisions in a 'matching' mode, the point can only
   * be used if coming from the relevant match
   * @param i_from index of the source (KDTree+values) of interest
   */
  bool checkRestrictionsForSource(const Point & pt,
                                  const unsigned int valid_mesh_div,
                                  const unsigned int i_from) const;

  /// KD-Trees for all the local source apps
  std::vector<std::shared_ptr<KDTree>> _local_kdtrees;

  /// KD-Trees for nodes nearest to a given position on each local source app
  std::vector<std::vector<std::shared_ptr<KDTree>>> _local_positions_kdtrees;

  /// All the nodes that meet the spatial restrictions in all the local source apps
  std::vector<std::vector<Point>> _local_points;

  /// Values of the variable being transferred at all the points in _local_points
  std::vector<std::vector<Real>> _local_values;

  /// Number of points to consider
  unsigned int _num_nearest_points;

  /// Whether to group data when creating the nearest-point regions
  const bool _group_subapps;

  /// Whether the source of the values is at nodes (true) or centroids (false) for each variable
  std::vector<bool> _source_is_nodes;

  /// Whether we can just use the local zero-indexed dof to get the value from the solution
  std::vector<bool> _use_zero_dof_for_value;

  /// Number of KD-Trees to create
  unsigned int _num_sources;
};
