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

#include <torch/torch.h>

#include <cstdint>
#include <vector>

/**
 * On-policy trajectory storage for fixed-horizon RL training.
 */
class LibtorchRLTrajectoryBuffer
{
public:
  struct Trajectory
  {
    /// Observations for each transition.
    std::vector<std::vector<Real>> observations;
    /// Next observations for each transition.
    std::vector<std::vector<Real>> next_observations;
    /// Actions applied at each transition.
    std::vector<std::vector<Real>> actions;
    /// Action log-probabilities recorded during rollout.
    std::vector<std::vector<Real>> log_probabilities;
    /// Scalar rewards for each transition.
    std::vector<Real> rewards;
    /// Critic targets aligned with each transition.
    std::vector<Real> value_targets;
    /// Advantage estimates aligned with each transition.
    std::vector<Real> advantages;
  };

  struct TensorBatch
  {
    /// Flattened observation matrix.
    torch::Tensor observations;
    /// Flattened next-observation matrix.
    torch::Tensor next_observations;
    /// Flattened action matrix.
    torch::Tensor actions;
    /// Flattened action log-probabilities.
    torch::Tensor log_probabilities;
    /// Flattened rewards.
    torch::Tensor rewards;
    /// Flattened critic targets.
    torch::Tensor value_targets;
    /// Flattened advantages.
    torch::Tensor advantages;

    /// Return the number of transitions represented by the batch.
    std::int64_t size() const { return observations.defined() ? observations.size(0) : 0; }
  };

  /**
   * Append one trajectory to the on-policy buffer.
   * @param trajectory Trajectory to store.
   */
  void addTrajectory(Trajectory trajectory);

  /// Clear every stored trajectory.
  void clear();

  bool empty() const { return _trajectories.empty(); }

  std::size_t numTrajectories() const { return _trajectories.size(); }

  /**
   * Count the total number of transitions stored across every trajectory.
   * @return Total transition count.
   */
  std::size_t numTransitions() const;

  std::vector<Trajectory> & trajectories() { return _trajectories; }
  const std::vector<Trajectory> & trajectories() const { return _trajectories; }

  /**
   * Flatten every stored trajectory into one tensor batch.
   * @return Tensor batch ready for mini-batch sampling.
   */
  TensorBatch flatten() const;

private:
  /**
   * Validate a trajectory before it is stored.
   * @param trajectory Trajectory to validate.
   */
  static void validateTrajectory(const Trajectory & trajectory);

  std::vector<Trajectory> _trajectories;
};

#endif
