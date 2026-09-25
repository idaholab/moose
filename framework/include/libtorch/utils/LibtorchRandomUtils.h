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

#include <torch/torch.h>

#include "MooseTypes.h"

namespace Moose
{

/// Create an owned CPU generator using libtorch's default seed behavior.
at::Generator makeLibtorchCPUGenerator();

/**
 * Create an owned CPU generator with an explicit seed.
 * @param seed Seed value passed to the libtorch CPU generator.
 */
at::Generator makeLibtorchCPUGenerator(uint64_t seed);

/**
 * Fill a tensor with a (semi) orthogonal matrix using the provided generator.
 * This mirrors torch::nn::init::orthogonal_, but avoids the ambient default RNG.
 * @param tensor Tensor to initialize in place.
 * @param gain Scaling factor applied after the orthogonal initialization.
 * @param generator Optional torch random-number generator used to sample the initialization.
 */
void orthogonalInitializeTensor(torch::Tensor & tensor,
                                Real gain = 1.0,
                                c10::optional<at::Generator> generator = c10::nullopt);

} // namespace Moose

#endif
