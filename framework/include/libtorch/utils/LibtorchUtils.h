//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include "MooseUtils.h"

namespace LibtorchUtils
{

/**
 * Utility function that converts a standard vector to a `torch::Tensor`.
 * @tparam DataType The type of data (float,double, etc.) which the vector is filled with
 * @param vector The vector that needs to be converted
 * @param tensor The output tensor
 * @param detach If the gradient information needs to be detached during the conversion
 */
template <typename DataType>
void
vectorToTensor(std::vector<DataType> & vector, torch::Tensor & tensor, const bool detach = false);

/**
 * Utility function that converts a `torch::Tensor` to a standard vector.
 * @tparam DataType The type of data (float,double, etc.) which the vector is filled with
 * @param tensor The tensor which needs to be converted
 * @param vector The output vector
 */
template <typename DataType>
void tensorToVector(torch::Tensor & tensor, std::vector<DataType> & vector);

} // LibtorchUtils namespace

#endif
