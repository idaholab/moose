//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchUtils.h"

namespace LibtorchUtils
{

template <typename DataType>
void
vectorToTensor(std::vector<DataType> & vector, torch::Tensor & tensor, const bool detach)
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

// Explicitly instantiate for DataType=Real
template void
vectorToTensor<Real>(std::vector<Real> & vector, torch::Tensor & tensor, const bool detach);

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
    mooseError(
        "Cannot cast tensor values to", MooseUtils::prettyCppType<DataType>(), "!\n", e.msg());
  }

  const auto & sizes = tensor.sizes();

  long int max_size = 0;
  for (const auto & dim_size : sizes)
    // We do this comparison because XCode complains if we use std::max
    max_size = dim_size > max_size ? dim_size : max_size;

  mooseAssert(max_size == tensor.numel(), "The given tensor should be one-dimensional!");
  vector = {tensor.data_ptr<DataType>(), tensor.data_ptr<DataType>() + tensor.numel()};
}

// Explicitly instantiate for DataType=Real
template void tensorToVector<Real>(torch::Tensor & tensor, std::vector<Real> & vector);

} // LibtorchUtils namespace

#endif
