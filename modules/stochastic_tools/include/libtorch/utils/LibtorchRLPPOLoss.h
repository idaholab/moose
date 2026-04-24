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

#include "LibtorchActorNeuralNet.h"
#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchRLMiniBatchSampler.h"

struct LibtorchRLPPOLossOutput
{
  /// Clipped actor loss for the current mini-batch.
  torch::Tensor actor_loss;
  /// Critic regression loss for the current mini-batch.
  torch::Tensor critic_loss;
  /// Mean action-distribution entropy for the current mini-batch.
  torch::Tensor entropy;
};

/**
 * PPO clipped surrogate loss on top of the reusable RL buffer/value-estimation core.
 */
class LibtorchRLPPOLoss
{
public:
  /**
   * Build the PPO loss helper.
   * @param clip_parameter PPO clipping width.
   * @param entropy_coeff Weight applied to the entropy bonus.
   */
  LibtorchRLPPOLoss(Real clip_parameter, Real entropy_coeff);

  /**
   * Compute actor, critic, and entropy terms for one mini-batch.
   * @param policy_network Actor network used for the policy term.
   * @param value_network Critic network used for the value term.
   * @param batch Mini-batch pulled from the on-policy trajectory buffer.
   * @return The three loss components for the mini-batch.
   */
  LibtorchRLPPOLossOutput compute(Moose::LibtorchActorNeuralNet & policy_network,
                                  Moose::LibtorchArtificialNeuralNet & value_network,
                                  const LibtorchRLMiniBatch & batch) const;

private:
  /**
   * Collapse multi-action log-probabilities or entropies into one column tensor.
   * @param tensor Action-wise tensor to reduce.
   * @return Column tensor with one value per row in the mini-batch.
   */
  static torch::Tensor reduceActionDimension(const torch::Tensor & tensor);

  const Real _clip_parameter;
  const Real _entropy_coeff;
};

#endif
