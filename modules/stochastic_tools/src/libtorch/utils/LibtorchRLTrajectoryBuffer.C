//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchRLTrajectoryBuffer.h"

#include "MooseError.h"

#include "libmesh/utility.h"

namespace
{

torch::Tensor
bufferMatrixToTensor(const std::vector<std::vector<Real>> & rows)
{
  if (rows.empty())
    return {};

  const auto num_columns = rows.front().size();
  auto tensor = torch::zeros({static_cast<long>(rows.size()), static_cast<long>(num_columns)},
                             torch::TensorOptions().dtype(torch::kDouble));
  auto accessor = tensor.accessor<Real, 2>();

  for (const auto row_i : make_range(rows.size()))
  {
    if (rows[row_i].size() != num_columns)
      mooseError("All rows must have the same number of entries when flattening an RL batch.");

    for (const auto column_i : make_range(num_columns))
      accessor[row_i][column_i] = rows[row_i][column_i];
  }

  return tensor;
}

torch::Tensor
bufferVectorToColumnTensor(const std::vector<Real> & values)
{
  if (values.empty())
    return {};

  auto tensor = torch::zeros({static_cast<long>(values.size()), 1},
                             torch::TensorOptions().dtype(torch::kDouble));
  auto accessor = tensor.accessor<Real, 2>();

  for (const auto value_i : make_range(values.size()))
    accessor[value_i][0] = values[value_i];

  return tensor;
}

} // namespace

void
LibtorchRLTrajectoryBuffer::addTrajectory(Trajectory trajectory)
{
  validateTrajectory(trajectory);
  _trajectories.push_back(std::move(trajectory));
}

void
LibtorchRLTrajectoryBuffer::clear()
{
  _trajectories.clear();
}

std::size_t
LibtorchRLTrajectoryBuffer::numTransitions() const
{
  std::size_t transitions = 0;
  for (const auto & trajectory : _trajectories)
    transitions += trajectory.rewards.size();
  return transitions;
}

LibtorchRLTrajectoryBuffer::TensorBatch
LibtorchRLTrajectoryBuffer::flatten() const
{
  TensorBatch batch;

  if (_trajectories.empty())
    return batch;

  std::vector<std::vector<Real>> observations;
  std::vector<std::vector<Real>> next_observations;
  std::vector<std::vector<Real>> actions;
  std::vector<std::vector<Real>> log_probabilities;
  std::vector<Real> rewards;
  std::vector<Real> value_targets;
  std::vector<Real> advantages;

  observations.reserve(numTransitions());
  next_observations.reserve(numTransitions());
  actions.reserve(numTransitions());
  log_probabilities.reserve(numTransitions());
  rewards.reserve(numTransitions());
  value_targets.reserve(numTransitions());
  advantages.reserve(numTransitions());

  for (const auto & trajectory : _trajectories)
  {
    if (!trajectory.value_targets.empty() &&
        trajectory.value_targets.size() != trajectory.rewards.size())
      mooseError("Value targets must match the reward length of the trajectory.");
    if (!trajectory.advantages.empty() && trajectory.advantages.size() != trajectory.rewards.size())
      mooseError("Advantages must match the reward length of the trajectory.");

    observations.insert(
        observations.end(), trajectory.observations.begin(), trajectory.observations.end());
    next_observations.insert(next_observations.end(),
                             trajectory.next_observations.begin(),
                             trajectory.next_observations.end());
    actions.insert(actions.end(), trajectory.actions.begin(), trajectory.actions.end());
    log_probabilities.insert(log_probabilities.end(),
                             trajectory.log_probabilities.begin(),
                             trajectory.log_probabilities.end());
    rewards.insert(rewards.end(), trajectory.rewards.begin(), trajectory.rewards.end());
    value_targets.insert(
        value_targets.end(), trajectory.value_targets.begin(), trajectory.value_targets.end());
    advantages.insert(advantages.end(), trajectory.advantages.begin(), trajectory.advantages.end());
  }

  batch.observations = bufferMatrixToTensor(observations);
  batch.next_observations = bufferMatrixToTensor(next_observations);
  batch.actions = bufferMatrixToTensor(actions);
  batch.log_probabilities = bufferMatrixToTensor(log_probabilities);
  batch.rewards = bufferVectorToColumnTensor(rewards);
  batch.value_targets = bufferVectorToColumnTensor(value_targets);
  batch.advantages = bufferVectorToColumnTensor(advantages);

  return batch;
}

void
LibtorchRLTrajectoryBuffer::validateTrajectory(const Trajectory & trajectory)
{
  const auto num_steps = trajectory.rewards.size();

  if (trajectory.observations.size() != num_steps)
    mooseError("RL trajectory observations must match the reward sequence length.");

  if (trajectory.next_observations.size() != num_steps)
    mooseError("RL trajectory next observations must match the reward sequence length.");

  if (trajectory.actions.size() != num_steps)
    mooseError("RL trajectory actions must match the reward sequence length.");

  if (trajectory.log_probabilities.size() != num_steps)
    mooseError("RL trajectory log probabilities must match the reward sequence length.");

  if (!trajectory.value_targets.empty() && trajectory.value_targets.size() != num_steps)
    mooseError("RL trajectory value targets must match the reward sequence length.");

  if (!trajectory.advantages.empty() && trajectory.advantages.size() != num_steps)
    mooseError("RL trajectory advantages must match the reward sequence length.");
}

#endif
