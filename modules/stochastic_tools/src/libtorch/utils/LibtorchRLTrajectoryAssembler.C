//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchRLTrajectoryAssembler.h"

#include "MooseError.h"

#include "libmesh/utility.h"

#include <numeric>

LibtorchRLTrajectoryAssembler::LibtorchRLTrajectoryAssembler(
    const unsigned int input_timesteps,
    const unsigned int timestep_window,
    const bool shift_outputs,
    const bool average_reward_over_timestep_window)
  : _timestep_window(timestep_window),
    _shift_outputs(shift_outputs),
    _average_reward_over_timestep_window(average_reward_over_timestep_window),
    _observation_history(input_timesteps)
{
  if (!_timestep_window)
    mooseError("The RL trajectory timestep window must be at least 1.");
}

std::vector<LibtorchRLTrajectoryBuffer::Trajectory>
LibtorchRLTrajectoryAssembler::assemble(const ReporterData & observations,
                                        const ReporterData & actions,
                                        const ReporterData & log_probabilities,
                                        const std::vector<std::vector<Real>> & rewards) const
{
  if (actions.size() != log_probabilities.size())
    mooseError("The number of log-probability reporters (",
               log_probabilities.size(),
               ") must match the number of action reporters (",
               actions.size(),
               ").");

  checkSampleCount(observations, "observation", rewards.size());
  checkSampleCount(actions, "action", rewards.size());
  checkSampleCount(log_probabilities, "log-probability", rewards.size());

  std::vector<LibtorchRLTrajectoryBuffer::Trajectory> trajectories;

  for (const auto sample_i : index_range(rewards))
  {
    const auto & reward_sample = rewards[sample_i];
    const auto num_transitions = numTransitions(reward_sample.size());
    if (!num_transitions)
      continue;

    std::vector<std::vector<Real>> component_trajectories(observations.size());
    for (const auto observation_i : index_range(observations))
      component_trajectories[observation_i] =
          downsample((*observations[observation_i])[sample_i], 0, num_transitions + 1);

    LibtorchRLTrajectoryBuffer::Trajectory trajectory;
    trajectory.observations.reserve(num_transitions);
    trajectory.next_observations.reserve(num_transitions);
    trajectory.actions.assign(num_transitions, std::vector<Real>());
    trajectory.log_probabilities.assign(num_transitions, std::vector<Real>());

    for (auto & action_row : trajectory.actions)
      action_row.reserve(actions.size());
    for (auto & log_probability_row : trajectory.log_probabilities)
      log_probability_row.reserve(log_probabilities.size());

    for (const auto step_i : make_range(num_transitions))
    {
      trajectory.observations.push_back(
          _observation_history.stackTrajectoryObservation(component_trajectories, step_i));
      trajectory.next_observations.push_back(
          _observation_history.stackTrajectoryObservation(component_trajectories, step_i + 1));
    }

    for (const auto action_i : index_range(actions))
    {
      const auto action_sequence =
          downsample((*actions[action_i])[sample_i], _shift_outputs, num_transitions);
      const auto log_probability_sequence =
          downsample((*log_probabilities[action_i])[sample_i], _shift_outputs, num_transitions);

      for (const auto step_i : make_range(num_transitions))
      {
        trajectory.actions[step_i].push_back(action_sequence[step_i]);
        trajectory.log_probabilities[step_i].push_back(log_probability_sequence[step_i]);
      }
    }

    trajectory.rewards = _average_reward_over_timestep_window
                             ? windowAverage(reward_sample, num_transitions)
                             : downsample(reward_sample, _timestep_window, num_transitions);
    // Full-solve rollout states are discarded after transfer, so GAE must stop at the boundary.
    trajectory.terminals.assign(num_transitions, false);
    trajectory.terminals.back() = true;

    trajectories.push_back(std::move(trajectory));
  }

  return trajectories;
}

unsigned int
LibtorchRLTrajectoryAssembler::numTransitions(const std::size_t raw_sequence_size) const
{
  // Raw index 0 holds the initial value, so each transition consumes one full window after it.
  return raw_sequence_size ? (raw_sequence_size - 1) / _timestep_window : 0;
}

void
LibtorchRLTrajectoryAssembler::checkSampleCount(const ReporterData & data,
                                                const std::string & quantity,
                                                const std::size_t num_samples)
{
  for (const auto component_i : index_range(data))
    if (data[component_i]->size() != num_samples)
      mooseError("Entry ",
                 component_i,
                 " of the ",
                 quantity,
                 " reporters holds ",
                 data[component_i]->size(),
                 " samples, but the reward reporter holds ",
                 num_samples,
                 " samples.");
}

std::vector<Real>
LibtorchRLTrajectoryAssembler::downsample(const std::vector<Real> & sample,
                                          const unsigned int offset,
                                          const unsigned int num_entries) const
{
  std::vector<Real> values;
  values.reserve(num_entries);

  for (const auto entry_i : make_range(num_entries))
  {
    const auto raw_index = offset + entry_i * _timestep_window;
    if (raw_index >= sample.size())
      mooseError("Reporter data is shorter than required by the configured timestep window and "
                 "history stacking.");
    values.push_back(sample[raw_index]);
  }

  return values;
}

std::vector<Real>
LibtorchRLTrajectoryAssembler::windowAverage(const std::vector<Real> & sample,
                                             const unsigned int num_entries) const
{
  std::vector<Real> values;
  values.reserve(num_entries);

  for (const auto entry_i : make_range(num_entries))
  {
    const auto window_begin = 1 + entry_i * _timestep_window;
    const auto window_end = window_begin + _timestep_window;
    if (window_end > sample.size())
      mooseError(
          "Reporter reward data is shorter than required by the configured timestep window.");

    const Real sum =
        std::accumulate(sample.begin() + window_begin, sample.begin() + window_end, 0.0);
    values.push_back(sum / _timestep_window);
  }

  return values;
}

#endif
