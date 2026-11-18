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
  /**
   * Vector data for one on-policy trajectory.
   */
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
    /// Terminal flags for each transition. A true value cuts value bootstrapping at that step.
    std::vector<bool> terminals;
    /// Critic targets aligned with each transition.
    std::vector<Real> value_targets;
    /// Advantage estimates aligned with each transition.
    std::vector<Real> advantages;
  };

  /**
   * Flattened tensor data assembled from the stored trajectories.
   */
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

    /**
     * Return the number of transitions represented by the batch.
     */
    std::int64_t size() const { return observations.defined() ? observations.size(0) : 0; }
  };

  /**
   * Reward statistics over the stored trajectories.
   */
  struct RewardStatistics
  {
    /// Mean reward over every stored transition.
    Real mean = 0.0;
    /// Standard deviation of every stored transition reward about the overall mean.
    Real std = 0.0;
    /// Mean reward of each non-empty trajectory.
    std::vector<Real> trajectory_means;
    /// Reward standard deviation of each non-empty trajectory about its own mean.
    std::vector<Real> trajectory_stds;
  };

  /**
   * Append one trajectory to the on-policy buffer.
   * @param trajectory Trajectory to store.
   */
  void addTrajectory(Trajectory trajectory);

  /**
   * Clear every stored trajectory.
   */
  void clear();

  /**
   * Return true if the buffer contains no trajectories.
   */
  bool empty() const { return _trajectories.empty(); }

  /**
   * Return the number of stored trajectories.
   */
  std::size_t numTrajectories() const { return _trajectories.size(); }

  /**
   * Count the total number of transitions stored across every trajectory.
   * @return Total transition count.
   */
  std::size_t numTransitions() const;

  /**
   * Return mutable access to the stored trajectories.
   */
  std::vector<Trajectory> & trajectories() { return _trajectories; }

  /**
   * Return read-only access to the stored trajectories.
   */
  const std::vector<Trajectory> & trajectories() const { return _trajectories; }

  /**
   * Flatten every stored trajectory into one tensor batch.
   * @return Tensor batch ready for mini-batch sampling.
   */
  TensorBatch flatten() const;

  /**
   * Compute reward statistics over the stored trajectories.
   * @return Overall and per-trajectory reward means and standard deviations.
   */
  RewardStatistics rewardStatistics() const;

private:
  /**
   * Validate a trajectory before it is stored.
   * @param trajectory Trajectory to validate.
   */
  static void validateTrajectory(const Trajectory & trajectory);

  /// Stored on-policy trajectories.
  std::vector<Trajectory> _trajectories;
};

#endif
