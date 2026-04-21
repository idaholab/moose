//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchRLMiniBatchSampler.h"

#include "MooseError.h"

#include <algorithm>

std::vector<LibtorchRLMiniBatch>
LibtorchRLMiniBatchSampler::sample(const LibtorchRLTrajectoryBuffer::TensorBatch & batch,
                                   const unsigned int batch_size,
                                   const bool standardize_advantage) const
{
  std::vector<LibtorchRLMiniBatch> mini_batches;

  if (!batch.size())
    return mini_batches;

  validateBatch(batch);

  const auto effective_batch_size = std::max<unsigned int>(1, batch_size);
  auto permutation = torch::randperm(batch.size(), torch::TensorOptions().dtype(torch::kLong));

  for (std::int64_t batch_begin = 0; batch_begin < batch.size();
       batch_begin += effective_batch_size)
  {
    const auto batch_end = std::min<std::int64_t>(
        batch.size(), batch_begin + static_cast<std::int64_t>(effective_batch_size));
    const auto indices = permutation.narrow(0, batch_begin, batch_end - batch_begin);
    mini_batches.push_back(makeMiniBatch(batch, indices, standardize_advantage));
  }

  return mini_batches;
}

void
LibtorchRLMiniBatchSampler::validateBatch(const LibtorchRLTrajectoryBuffer::TensorBatch & batch)
{
  if (!batch.actions.defined() || !batch.log_probabilities.defined() ||
      !batch.value_targets.defined() || !batch.advantages.defined())
    mooseError("RL tensor batches must define observations, actions, log probabilities, value "
               "targets, and advantages before mini-batch sampling.");

  const auto batch_size = batch.size();
  const auto validate_rows = [batch_size](const torch::Tensor & tensor, const char * name)
  {
    if (!tensor.defined() || tensor.size(0) != batch_size)
      mooseError(
          "RL tensor batch field ", name, " must have the same number of rows as observations.");
  };

  validate_rows(batch.actions, "actions");
  validate_rows(batch.log_probabilities, "log_probabilities");
  validate_rows(batch.value_targets, "value_targets");
  validate_rows(batch.advantages, "advantages");
}

LibtorchRLMiniBatch
LibtorchRLMiniBatchSampler::makeMiniBatch(const LibtorchRLTrajectoryBuffer::TensorBatch & batch,
                                          const torch::Tensor & indices,
                                          const bool standardize_advantage)
{
  LibtorchRLMiniBatch mini_batch;

  mini_batch.observations = batch.observations.index({indices});
  mini_batch.actions = batch.actions.index({indices});
  mini_batch.old_log_probabilities = batch.log_probabilities.index({indices});
  mini_batch.value_targets = batch.value_targets.index({indices});
  mini_batch.advantages = batch.advantages.index({indices});

  if (standardize_advantage)
  {
    const auto std = mini_batch.advantages.std(false);
    mini_batch.advantages = (mini_batch.advantages - mini_batch.advantages.mean()) / (std + 1e-10);
  }

  return mini_batch;
}

#endif
