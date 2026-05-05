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
void vectorToTensor(const std::vector<DataType> & vector,
                    torch::Tensor & tensor,
                    const bool detach = false);

/**
 * Utility function that creates an owning tensor copy of a standard vector.
 * @tparam DataType The vector element type
 * @param vector The vector that needs to be copied
 * @param sizes The desired tensor shape
 */
template <typename DataType>
torch::Tensor vectorToTensorCopy(const std::vector<DataType> & vector, c10::IntArrayRef sizes);

/**
 * Utility function that creates a non-owning tensor view of a standard vector.
 * The returned tensor shares the mutable storage of the provided vector, so the
 * vector must outlive the tensor and may be modified through the tensor.
 * @tparam DataType The vector element type
 * @param vector The vector that needs to be wrapped
 * @param sizes The desired tensor shape
 */
template <typename DataType>
torch::Tensor vectorToTensorView(std::vector<DataType> & vector, c10::IntArrayRef sizes);

/**
 * Move a tensor to the configured libtorch device.
 * @param tensor The tensor to move
 * @param device_type The target torch device type
 */
void moveToLibtorchDevice(torch::Tensor & tensor, const torch::DeviceType device_type);

/**
 * Return a detached contiguous CPU copy of a tensor.
 *
 * This is for call sites that read tensor storage through CPU accessors or data_ptr().
 * Moving a tensor to CPU does not guarantee that logical tensor order is backed by a dense
 * linear memory layout; contiguous() makes that invariant explicit.
 *
 * @param tensor The tensor to copy to CPU
 */
torch::Tensor toCPUContiguous(const torch::Tensor & tensor);

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
