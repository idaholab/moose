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
  torch::Tensor actor_loss;
  torch::Tensor critic_loss;
  torch::Tensor entropy;
};

/**
 * PPO clipped surrogate loss on top of the reusable RL buffer/value-estimation core.
 */
class LibtorchRLPPOLoss
{
public:
  LibtorchRLPPOLoss(Real clip_parameter, Real entropy_coeff);

  LibtorchRLPPOLossOutput compute(Moose::LibtorchActorNeuralNet & policy_network,
                                  Moose::LibtorchArtificialNeuralNet & value_network,
                                  const LibtorchRLMiniBatch & batch) const;

private:
  static torch::Tensor reduceActionDimension(const torch::Tensor & tensor);

  const Real _clip_parameter;
  const Real _entropy_coeff;
};

#endif
