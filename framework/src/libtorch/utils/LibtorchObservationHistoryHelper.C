//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchObservationHistoryHelper.h"

#include "MooseError.h"

#include <algorithm>
#include "libmesh/utility.h"

LibtorchObservationHistoryHelper::LibtorchObservationHistoryHelper(
    const unsigned int input_timesteps)
  : _input_timesteps(input_timesteps)
{
  if (_input_timesteps == 0)
    mooseError("Observation history requires at least one input timestep.");
}

void
LibtorchObservationHistoryHelper::validateTrajectoryShape(
    const std::vector<std::vector<Real>> & component_trajectories) const
{
  if (component_trajectories.empty())
    return;

  const auto trajectory_size = component_trajectories.front().size();
  for (const auto & trajectory : component_trajectories)
    if (trajectory.size() != trajectory_size)
      mooseError("Observation trajectories must all have the same number of timesteps.");
}

void
LibtorchObservationHistoryHelper::initializeHistory(
    const std::vector<Real> & observation, std::vector<std::vector<Real>> & old_observations) const
{
  old_observations.assign(_input_timesteps - 1, observation);
}

void
LibtorchObservationHistoryHelper::advanceHistory(
    const std::vector<Real> & observation, std::vector<std::vector<Real>> & old_observations) const
{
  if (old_observations.empty())
    return;

  std::rotate(old_observations.rbegin(), old_observations.rbegin() + 1, old_observations.rend());
  old_observations[0] = observation;
}

std::vector<Real>
LibtorchObservationHistoryHelper::expandObservationFactors(
    const std::vector<Real> & observation_factors) const
{
  if (observation_factors.empty())
    return {};

  std::vector<Real> expanded;
  expanded.reserve(observation_factors.size() * _input_timesteps);

  for (const auto lag : make_range(_input_timesteps))
  {
    libmesh_ignore(lag);
    expanded.insert(expanded.end(), observation_factors.begin(), observation_factors.end());
  }

  return expanded;
}

std::vector<Real>
LibtorchObservationHistoryHelper::stackCurrentObservation(
    const std::vector<Real> & observation,
    const std::vector<std::vector<Real>> & old_observations) const
{
  const auto expected_history_size = _input_timesteps - 1;
  if (old_observations.size() != expected_history_size)
    mooseError("Observation history must contain ",
               expected_history_size,
               " stored entries, but got ",
               old_observations.size(),
               ".");

  std::vector<Real> stacked;
  stacked.reserve(observation.size() * _input_timesteps);

  stacked.insert(stacked.end(), observation.begin(), observation.end());

  for (const auto & history_entry : old_observations)
  {
    if (history_entry.size() != observation.size())
      mooseError("Observation history entries must have the same size as the current "
                 "observation.");
    stacked.insert(stacked.end(), history_entry.begin(), history_entry.end());
  }

  return stacked;
}

std::vector<Real>
LibtorchObservationHistoryHelper::stackTrajectoryObservation(
    const std::vector<std::vector<Real>> & component_trajectories,
    const unsigned int time_index) const
{
  validateTrajectoryShape(component_trajectories);

  if (component_trajectories.empty())
    return {};

  const auto trajectory_size = component_trajectories.front().size();
  if (time_index >= trajectory_size)
    mooseError("Requested observation time index is out of range.");

  std::vector<Real> stacked;
  stacked.reserve(component_trajectories.size() * _input_timesteps);

  for (const auto lag : make_range(_input_timesteps))
  {
    const auto source_index = time_index > lag ? time_index - lag : 0;
    for (const auto component_i : make_range(component_trajectories.size()))
      stacked.push_back(component_trajectories[component_i][source_index]);
  }

  return stacked;
}

#endif
