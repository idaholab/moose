//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Base class for working with KDTrees in transfers, whether for interpolation or extrapolation
 */
class MultiAppGeneralFieldKDTreeTransferBase : public MultiAppGeneralFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldKDTreeTransferBase(const InputParameters & parameters);

  void initialSetup() override;

  // Use solution invalid output for these warnings
  usingCombinedWarningSolutionWarnings;

protected:
  virtual void prepareEvaluationOfInterpValues(const unsigned int var_index) override;

  using MultiAppGeneralFieldTransfer::inBlocks;
  bool inBlocks(const std::set<SubdomainID> & blocks,
                const MooseMesh & mesh,
                const Elem * elem) const override;

  bool usesMooseAppCoordTransform() const override { return true; }
  /*
   * Build KD-Trees for each local app
   * @param var_index the index of the variable being transferred
   * @details fills _local_kdtrees, _local_points and _local_values
   *          Indexing is: local apps (outer-indexing) OR positions (if using nearest_positions),
   *          local nodes (inner-indexing)
   */
  virtual void buildKDTrees(const unsigned int var_index) = 0;

  /// Pre-compute the number of sources
  /// Number of KDTrees used to hold the locations and variable value data
  void computeNumSources();

  /**
   * Get the index of the app when inside of a KD-Tree source loop, where multiple applications
   * could be lumped (grouped) inside the same KD-Tree
   * @param kdtree_index index of the kd-tree / source
   * @param app_index_in_tree index of the application within the multiple apps contributing values
   * to a KD-tree. This is a local index
   */
  unsigned int getAppIndex(unsigned int kdtree_index, unsigned int app_index_in_tree) const;

  /// Number of applications which contributed nearest-locations to each KD-tree
  unsigned int getNumAppsPerTree() const;

  /// Number of divisions (nearest-positions or source mesh divisions) used when building KD-Trees
  unsigned int getNumDivisions() const;

  /**
   * Transform a point into the frame used for KD-tree queries.
   * When grouping subapps with nearest-positions, KD-trees are built in the global frame so this
   * returns pt unchanged. Otherwise delegates to getPointInSourceAppFrame.
   * @param i_source index of the KD-tree source (may be a position index when grouping subapps)
   * @param pt the point in the global frame
   */
  Point getPointInSourceKDTreeFrame(unsigned int i_source, const Point & pt) const;

  /**
   * @brief Examine all spatial restrictions that could preclude this source from being
   *        a valid source for this point
   * @param pt point of interest
   * @param valid_mesh_div if using source mesh divisions in a 'matching' mode, the point can only
   * be used if coming from the relevant match
   * @param i_from index of the source (= a KDTree+values) of interest. Local index, goes from 0 to
   * the number of sources - 1.
   */
  bool checkRestrictionsForSource(const Point & pt,
                                  const unsigned int valid_mesh_div,
                                  const unsigned int i_from) const;

  /**
   * @brief Search all local KD-trees for the nearest node/element and update outgoing_val.
   *
   * Performs a first pass to find the nearest neighbor value across all sources, then a second
   * pass for search-value-conflict detection when _search_value_conflicts is true.
   *
   * @param pt           Target point in the reference coordinate frame
   * @param source_index     Mesh-division / sub-app index encoded in the point request
   * @param outgoing_val Accumulator {value, distance}; caller must pre-set .second to max()
   * @param point_found  Set to true when at least one non-empty KD-tree contributes a value
   */
  void evaluateNearestNodeFromKDTrees(const Point & pt,
                                      unsigned int mesh_div,
                                      std::pair<Real, Real> & outgoing_val,
                                      bool & point_found);

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

  /// Number of KD-Trees to create
  unsigned int _num_sources;
};
