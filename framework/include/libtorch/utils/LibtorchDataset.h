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
#include "MooseError.h"

namespace Moose
{

/**
 * This class is a wrapper around a libtorch dataset which can be used by the
 * data loaders in the neural net training process.
 */
class LibtorchDataset : public torch::data::datasets::Dataset<LibtorchDataset>
{
public:
  /// Construct using the input and output tensors
  LibtorchDataset(torch::Tensor dt, torch::Tensor rt) : _data_tensor(dt), _response_tensor(rt) {}

  /// Get a sample pair from the input and output tensors
  torch::data::Example<> get(size_t index) override
  {
    mooseAssert(index < size(), "Index is out of range!");
    return {_data_tensor[index], _response_tensor[index]};
  }

  /// Return the number of samples this data set contains
  torch::optional<size_t> size() const override
  {
    mooseAssert(_response_tensor.sizes().size(), "The tensors are empty!");
    return _response_tensor.sizes()[0];
  }

private:
  /// Tensor containing the data (inputs)
  torch::Tensor _data_tensor;
  /// Tensor containing the responses (outputs) for the data
  torch::Tensor _response_tensor;
};

}

#endif
