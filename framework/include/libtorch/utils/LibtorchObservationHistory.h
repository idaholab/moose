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
 * Shared observation normalization and history stacking logic for libtorch-based controls and
 * trainers.
 */
class LibtorchObservationHistory
{
public:
  LibtorchObservationHistory(unsigned int input_timesteps,
                             const std::vector<Real> & shift_factors = {},
                             const std::vector<Real> & scaling_factors = {});

  unsigned int inputTimesteps() const { return _input_timesteps; }

  std::vector<Real> normalize(const std::vector<Real> & observation) const;

  void normalizeInPlace(std::vector<Real> & observation) const;

  void normalizeTrajectoryInPlace(std::vector<std::vector<Real>> & observation_trajectories) const;

  void initializeHistory(const std::vector<Real> & normalized_observation,
                         std::vector<std::vector<Real>> & old_observations) const;

  void advanceHistory(const std::vector<Real> & normalized_observation,
                      std::vector<std::vector<Real>> & old_observations) const;

  std::vector<Real> expandFeatureFactors(const std::vector<Real> & feature_factors) const;

  std::vector<Real>
  stackCurrentObservation(const std::vector<Real> & normalized_observation,
                          const std::vector<std::vector<Real>> & old_observations) const;

  std::vector<Real> stackTrajectoryObservation(
      const std::vector<std::vector<Real>> & normalized_observation_trajectories,
      unsigned int time_index) const;

private:
  void validateFeatureCount(std::size_t feature_count) const;
  void validateTrajectoryShape(
      const std::vector<std::vector<Real>> & normalized_observation_trajectories) const;

  const unsigned int _input_timesteps;
  const std::vector<Real> _shift_factors;
  const std::vector<Real> _scaling_factors;
};

#endif
