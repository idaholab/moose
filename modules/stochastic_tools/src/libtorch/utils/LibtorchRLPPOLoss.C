//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchRLPPOLoss.h"

LibtorchRLPPOLoss::LibtorchRLPPOLoss(const Real clip_parameter, const Real entropy_coeff)
  : _clip_parameter(clip_parameter), _entropy_coeff(entropy_coeff)
{
}

LibtorchRLPPOLossOutput
LibtorchRLPPOLoss::compute(Moose::LibtorchActorNeuralNet & policy_network,
                           Moose::LibtorchArtificialNeuralNet & value_network,
                           const LibtorchRLMiniBatch & batch) const
{
  auto observations = batch.observations;
  policy_network.evaluate(observations, false);

  const auto current_log_probability =
      reduceActionDimension(policy_network.logProbability(batch.actions));
  const auto previous_log_probability = reduceActionDimension(batch.old_log_probabilities);
  const auto entropy = reduceActionDimension(policy_network.entropy());

  const auto ratio = (current_log_probability - previous_log_probability).exp();
  const auto surr1 = ratio * batch.advantages;
  const auto surr2 =
      torch::clamp(ratio, 1.0 - _clip_parameter, 1.0 + _clip_parameter) * batch.advantages;

  LibtorchRLPPOLossOutput output;
  output.actor_loss = -(torch::min(surr1, surr2) + _entropy_coeff * entropy).mean();
  output.critic_loss =
      torch::mse_loss(value_network.forward(batch.observations), batch.value_targets);
  output.entropy = entropy.mean();
  return output;
}

torch::Tensor
LibtorchRLPPOLoss::reduceActionDimension(const torch::Tensor & tensor)
{
  if (!tensor.defined())
    return tensor;

  if (tensor.dim() == 1)
    return tensor.unsqueeze(1);

  if (tensor.size(-1) == 1)
    return tensor;

  return tensor.sum(-1, true);
}

#endif
