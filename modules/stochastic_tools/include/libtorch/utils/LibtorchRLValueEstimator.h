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

#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchRLTrajectoryBuffer.h"

#include <vector>

/**
 * Computes GAE advantages and value targets for an on-policy trajectory buffer.
 */
class LibtorchRLValueEstimator
{
public:
  struct Targets
  {
    /// Generalized-advantage estimates.
    std::vector<Real> advantages;
    /// Critic regression targets.
    std::vector<Real> value_targets;
  };

  /**
   * Build the GAE helper.
   * @param discount_factor Reward discount factor.
   * @param lambda_factor GAE lambda factor.
   */
  LibtorchRLValueEstimator(Real discount_factor, Real lambda_factor);

  /**
   * Fill every trajectory in the buffer with value targets and advantages.
   * @param buffer On-policy trajectory buffer to update.
   * @param value_network Critic used for target estimation.
   */
  void computeValueTargets(LibtorchRLTrajectoryBuffer & buffer,
                           Moose::LibtorchArtificialNeuralNet & value_network) const;

  /**
   * Compute value targets and advantages for one trajectory.
   * @param trajectory Trajectory to evaluate.
   * @param value_network Critic used for target estimation.
   * @return Advantage and value-target vectors for the trajectory.
   */
  Targets estimate(const LibtorchRLTrajectoryBuffer::Trajectory & trajectory,
                   Moose::LibtorchArtificialNeuralNet & value_network) const;

private:
  /**
   * Evaluate the critic on a batch of observations.
   * @param observations Observation matrix to feed through the critic.
   * @param value_network Critic used for the evaluation.
   * @return One value estimate per observation row.
   */
  std::vector<Real> evaluate(const std::vector<std::vector<Real>> & observations,
                             Moose::LibtorchArtificialNeuralNet & value_network) const;

  const Real _discount_factor;
  const Real _lambda_factor;
};

#endif
