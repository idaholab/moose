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

#include "LibtorchRLTrajectoryBuffer.h"

#include <torch/torch.h>

#include <cstdint>
#include <vector>

struct LibtorchRLMiniBatch
{
  /// Flattened observation rows for the mini-batch.
  torch::Tensor observations;
  /// Action rows that match the sampled observations.
  torch::Tensor actions;
  /// Behavior-policy log probabilities for the sampled actions.
  torch::Tensor old_log_probabilities;
  /// Critic targets aligned with the sampled observations.
  torch::Tensor value_targets;
  /// Advantage estimates aligned with the sampled observations.
  torch::Tensor advantages;

  /// Return the number of rows stored in the mini-batch.
  std::int64_t size() const { return observations.defined() ? observations.size(0) : 0; }
};

/**
 * Samples shuffled mini-batches from a flattened on-policy trajectory batch.
 */
class LibtorchRLMiniBatchSampler
{
public:
  /**
   * Shuffle a flattened rollout batch into PPO-sized chunks.
   * @param batch Flattened rollout tensors ready for PPO updates.
   * @param batch_size Preferred number of rows per mini-batch.
   * @param standardize_advantage Whether to normalize the advantages inside each chunk.
   * @return Vector of sampled mini-batches.
   */
  std::vector<LibtorchRLMiniBatch>
  sample(const LibtorchRLTrajectoryBuffer::TensorBatch & batch,
         unsigned int batch_size,
         bool standardize_advantage,
         c10::optional<at::Generator> generator = c10::nullopt) const;

private:
  /**
   * Sanity-check that the flattened rollout tensors all line up.
   * @param batch Flattened rollout tensors to validate.
   */
  static void validateBatch(const LibtorchRLTrajectoryBuffer::TensorBatch & batch);

  /**
   * Slice one shuffled mini-batch out of the flattened rollout tensors.
   * @param batch Flattened rollout tensors.
   * @param indices Row indices assigned to this mini-batch.
   * @param standardize_advantage Whether to normalize the advantages in this slice.
   * @return One PPO mini-batch.
   */
  static LibtorchRLMiniBatch makeMiniBatch(const LibtorchRLTrajectoryBuffer::TensorBatch & batch,
                                           const torch::Tensor & indices,
                                           bool standardize_advantage);
};

#endif
