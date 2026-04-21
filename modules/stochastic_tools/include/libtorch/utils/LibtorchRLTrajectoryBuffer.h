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
    std::vector<std::vector<Real>> observations;
    std::vector<std::vector<Real>> next_observations;
    std::vector<std::vector<Real>> actions;
    std::vector<std::vector<Real>> log_probabilities;
    std::vector<Real> rewards;
    std::vector<Real> value_targets;
    std::vector<Real> advantages;
  };

  struct TensorBatch
  {
    torch::Tensor observations;
    torch::Tensor next_observations;
    torch::Tensor actions;
    torch::Tensor log_probabilities;
    torch::Tensor rewards;
    torch::Tensor value_targets;
    torch::Tensor advantages;

    std::int64_t size() const { return observations.defined() ? observations.size(0) : 0; }
  };

  void addTrajectory(Trajectory trajectory);

  void clear();

  bool empty() const { return _trajectories.empty(); }

  std::size_t numTrajectories() const { return _trajectories.size(); }

  std::size_t numTransitions() const;

  std::vector<Trajectory> & trajectories() { return _trajectories; }
  const std::vector<Trajectory> & trajectories() const { return _trajectories; }

  TensorBatch flatten() const;

private:
  static void validateTrajectory(const Trajectory & trajectory);

  std::vector<Trajectory> _trajectories;
};

#endif
