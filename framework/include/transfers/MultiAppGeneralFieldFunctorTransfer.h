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
#include "NonADFunctorInterface.h"

/**
 * Transfers a functor (can be variable, function, functor material property, spatial UO, or PP)
 * Inside the domain of definition: evaluates the functor with the specified argument
 * Outside the domain of definition: uses a user-specified extrapolation behavior
 */
class MultiAppGeneralFieldFunctorTransfer : public MultiAppGeneralFieldTransfer,
                                            public NonADFunctorInterface

{
public:
  static InputParameters validParams();

  MultiAppGeneralFieldFunctorTransfer(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void prepareEvaluationOfInterpValues(const unsigned int var_index) override;

  // TODO: rename! We are not just interpolating anymore
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
   * Build KD-Trees for each local app for the external values to extrapolate with
   * @param var_index the index of the variable being transferred
   * @details fills _local_kdtrees, _local_points and _local_values
   *          Indexing is: local apps (outer-indexing) OR positions (if using nearest_positions),
   *          local nodes (inner-indexing)
   */
  void buildKDTrees(const unsigned int var_index);

  /*
   * Evaluate values (interpolation and extrapolation) for incoming points
   * @param incoming_points all the points at which we need values
   * @param outgoing_vals vector containing the values and distances from point to nearest node
   */
  void evaluateValues(const std::vector<std::pair<Point, unsigned int>> & incoming_points,
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

  /// Names of the source functors
  const std::vector<MooseFunctorName> _functor_names;
  /// Pointers to the source functors
  std::vector<const Moose::Functor<Real> *> _functors;

  /// Extrapolation behavior
  const MooseEnum _extrapolation_behavior;

  /// Whether to group data when creating the nearest-point regions
  const bool _group_subapps;
  /// Number of KD-Trees to create
  unsigned int _num_sources;
};
