//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include "MooseTypes.h"

#include <vector>

/**
 * Shared observation history stacking and factor-expansion logic for libtorch-based controls and
 * trainers.
 */
class LibtorchObservationHistoryHelper
{
public:
  /**
   * Build an observation-history helper for libtorch inputs.
   * @param input_timesteps Number of timesteps to stack into each flattened input.
   */
  LibtorchObservationHistoryHelper(unsigned int input_timesteps);

  /// Return the number of timesteps stacked into each flattened input.
  unsigned int inputTimesteps() const { return _input_timesteps; }

  /**
   * Fill the history buffer with copies of the current observation.
   * @param observation Current observation.
   * @param old_observations History buffer that stores previous observations.
   */
  void initializeHistory(const std::vector<Real> & observation,
                         std::vector<std::vector<Real>> & old_observations) const;

  /**
   * Advance the history buffer by inserting the latest observation.
   * @param observation Current observation.
   * @param old_observations History buffer ordered from newest to oldest.
   */
  void advanceHistory(const std::vector<Real> & observation,
                      std::vector<std::vector<Real>> & old_observations) const;

  /**
   * Repeat per-observation-entry factors across all stacked timesteps.
   * @param observation_factors Per-entry factors for one observation vector.
   */
  std::vector<Real> expandObservationFactors(const std::vector<Real> & observation_factors) const;

  /**
   * Flatten the current observation together with its stored history.
   * @param observation Current observation.
   * @param old_observations History buffer ordered from newest to oldest.
   */
  std::vector<Real>
  stackCurrentObservation(const std::vector<Real> & observation,
                          const std::vector<std::vector<Real>> & old_observations) const;

  /**
   * Flatten one time slice of observation-component trajectories with causal history.
   * This uses [component][time] because the trainer receives reporter data one observation
   * component at a time, so keeping that layout avoids building an extra transposed
   * [time][component] container before stacking.
   * @param component_trajectories Observation trajectories indexed as [component][time].
   * @param time_index Time index to stack.
   */
  std::vector<Real>
  stackTrajectoryObservation(const std::vector<std::vector<Real>> & component_trajectories,
                             unsigned int time_index) const;

private:
  /// Check that all observation-component trajectories have a consistent shape.
  void validateTrajectoryShape(const std::vector<std::vector<Real>> & component_trajectories) const;

  /// Number of timesteps stacked into each flattened observation.
  const unsigned int _input_timesteps;
};

#endif
