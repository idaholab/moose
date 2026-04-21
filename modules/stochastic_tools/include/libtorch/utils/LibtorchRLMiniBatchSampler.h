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
  torch::Tensor observations;
  torch::Tensor actions;
  torch::Tensor old_log_probabilities;
  torch::Tensor value_targets;
  torch::Tensor advantages;

  std::int64_t size() const { return observations.defined() ? observations.size(0) : 0; }
};

/**
 * Samples shuffled mini-batches from a flattened on-policy trajectory batch.
 */
class LibtorchRLMiniBatchSampler
{
public:
  std::vector<LibtorchRLMiniBatch> sample(const LibtorchRLTrajectoryBuffer::TensorBatch & batch,
                                          unsigned int batch_size,
                                          bool standardize_advantage) const;

private:
  static void validateBatch(const LibtorchRLTrajectoryBuffer::TensorBatch & batch);

  static LibtorchRLMiniBatch makeMiniBatch(const LibtorchRLTrajectoryBuffer::TensorBatch & batch,
                                           const torch::Tensor & indices,
                                           bool standardize_advantage);
};

#endif
