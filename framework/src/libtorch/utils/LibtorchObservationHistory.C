//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchObservationHistory.h"

#include "MooseError.h"

#include <algorithm>
#include "libmesh/utility.h"

LibtorchObservationHistory::LibtorchObservationHistory(
    const unsigned int input_timesteps,
    const std::vector<Real> & shift_factors,
    const std::vector<Real> & scaling_factors)
  : _input_timesteps(input_timesteps),
    _shift_factors(shift_factors),
    _scaling_factors(scaling_factors.empty() ? std::vector<Real>(shift_factors.size(), 1.0)
                                             : scaling_factors)
{
  if (!_shift_factors.empty() && _shift_factors.size() != _scaling_factors.size())
    mooseError("Observation shift and scaling factors must have the same size.");
}

void
LibtorchObservationHistory::validateFeatureCount(const std::size_t feature_count) const
{
  if (!_shift_factors.empty() && feature_count != _shift_factors.size())
    mooseError("Observation feature count does not match the configured normalization factors.");
}

void
LibtorchObservationHistory::validateTrajectoryShape(
    const std::vector<std::vector<Real>> & normalized_response_trajectories) const
{
  if (normalized_response_trajectories.empty())
    return;

  validateFeatureCount(normalized_response_trajectories.size());

  const auto trajectory_size = normalized_response_trajectories.front().size();
  for (const auto & trajectory : normalized_response_trajectories)
    if (trajectory.size() != trajectory_size)
      mooseError("Observation trajectories must all have the same number of timesteps.");
}

std::vector<Real>
LibtorchObservationHistory::normalize(const std::vector<Real> & response) const
{
  auto normalized = response;
  normalizeInPlace(normalized);
  return normalized;
}

void
LibtorchObservationHistory::normalizeInPlace(std::vector<Real> & response) const
{
  validateFeatureCount(response.size());

  if (_shift_factors.empty())
    return;

  for (const auto i : make_range(response.size()))
    response[i] = (response[i] - _shift_factors[i]) * _scaling_factors[i];
}

void
LibtorchObservationHistory::normalizeTrajectoryInPlace(
    std::vector<std::vector<Real>> & response_trajectories) const
{
  validateTrajectoryShape(response_trajectories);

  if (_shift_factors.empty())
    return;

  for (const auto feature_i : make_range(response_trajectories.size()))
    for (auto & value : response_trajectories[feature_i])
      value = (value - _shift_factors[feature_i]) * _scaling_factors[feature_i];
}

void
LibtorchObservationHistory::initializeHistory(const std::vector<Real> & normalized_response,
                                              std::vector<std::vector<Real>> & old_responses) const
{
  old_responses.assign(_input_timesteps > 0 ? _input_timesteps - 1 : 0, normalized_response);
}

void
LibtorchObservationHistory::advanceHistory(const std::vector<Real> & normalized_response,
                                           std::vector<std::vector<Real>> & old_responses) const
{
  if (old_responses.empty())
    return;

  std::rotate(old_responses.rbegin(), old_responses.rbegin() + 1, old_responses.rend());
  old_responses[0] = normalized_response;
}

std::vector<Real>
LibtorchObservationHistory::expandFeatureFactors(const std::vector<Real> & feature_factors) const
{
  if (feature_factors.empty())
    return {};

  std::vector<Real> expanded;
  expanded.reserve(feature_factors.size() * _input_timesteps);

  for (const auto lag : make_range(_input_timesteps))
  {
    libmesh_ignore(lag);
    expanded.insert(expanded.end(), feature_factors.begin(), feature_factors.end());
  }

  return expanded;
}

std::vector<Real>
LibtorchObservationHistory::stackCurrentObservation(
    const std::vector<Real> & normalized_response,
    const std::vector<std::vector<Real>> & old_responses) const
{
  validateFeatureCount(normalized_response.size());

  std::vector<Real> stacked;
  stacked.reserve(normalized_response.size() * _input_timesteps);

  stacked.insert(stacked.end(), normalized_response.begin(), normalized_response.end());

  for (const auto history_i : make_range(_input_timesteps > 0 ? _input_timesteps - 1 : 0))
  {
    const auto & history_entry =
        history_i < old_responses.size() ? old_responses[history_i] : normalized_response;
    if (history_entry.size() != normalized_response.size())
      mooseError("Observation history entries must have the same feature size as the current "
                 "observation.");
    stacked.insert(stacked.end(), history_entry.begin(), history_entry.end());
  }

  return stacked;
}

std::vector<Real>
LibtorchObservationHistory::stackTrajectoryObservation(
    const std::vector<std::vector<Real>> & normalized_response_trajectories,
    const unsigned int time_index) const
{
  validateTrajectoryShape(normalized_response_trajectories);

  if (normalized_response_trajectories.empty())
    return {};

  const auto trajectory_size = normalized_response_trajectories.front().size();
  if (time_index >= trajectory_size)
    mooseError("Requested observation time index is out of range.");

  std::vector<Real> stacked;
  stacked.reserve(normalized_response_trajectories.size() * _input_timesteps);

  for (const auto lag : make_range(_input_timesteps))
  {
    const auto source_index = time_index > lag ? time_index - lag : 0;
    for (const auto feature_i : make_range(normalized_response_trajectories.size()))
      stacked.push_back(normalized_response_trajectories[feature_i][source_index]);
  }

  return stacked;
}

#endif
