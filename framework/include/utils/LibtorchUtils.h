//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef LIBTORCH_ENABLED
#include <torch/torch.h>

#include "MooseUtils.h"

namespace LibtorchUtils
{

template <typename DataType>
void
vectorToTensor(std::vector<DataType> & vector, torch::Tensor & tensor, const bool detach = false)
{
  auto options = torch::TensorOptions();
  if constexpr (std::is_same<DataType, double>::value)
    options = torch::TensorOptions().dtype(at::kDouble);
  else if constexpr (std::is_same<DataType, float>::value)
    options = torch::TensorOptions().dtype(at::kFloat);
  else
    static_assert(Moose::always_false<DataType>,
                  "vectorToTensor is not implemented for the given data type!");

  // We need to clone here because from_blob() doesn't take ownership of the pointer so if it
  // vector goes out of scope before tensor, we get unwanted behavior
  tensor = torch::from_blob(vector.data(), {long(vector.size()), 1}, options).clone();

  if (detach)
    tensor.detach();
}

template <typename DataType>
void
tensorToVector(torch::Tensor & tensor, std::vector<DataType> & vector)
{
  try
  {
    tensor.data_ptr<DataType>();
  }
  catch (const c10::Error & e)
  {
    mooseError("Cannot cast tensor values to type given for the vector! " + e.msg());
  }

  const auto & sizes = tensor.sizes();

  long int max_size = 0;
  long int product_size = 1;

  for (const auto & dim_size : sizes)
  {
    max_size = std::max(dim_size, max_size);
    product_size *= dim_size;
  }

  mooseAssert(max_size == product_size, "The given tensor should be one-dimensional!");
  vector = {tensor.data_ptr<Real>(), tensor.data_ptr<Real>() + max_size};
}

} // LibtorchUtils namespace

#endif
