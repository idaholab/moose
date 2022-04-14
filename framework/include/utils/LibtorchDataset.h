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

namespace Moose
{

class LibtorchDataset : public torch::data::datasets::Dataset<LibtorchDataset>
{
public:
  LibtorchDataset(torch::Tensor dt, torch::Tensor rt) : _data_tensor(dt), _response_tensor(rt) {}
  torch::data::Example<> get(size_t index) override
  {
    return {_data_tensor[index], _response_tensor[index]};
  }

  torch::optional<size_t> size() const override { return _response_tensor.sizes()[0]; }

private:
  torch::Tensor _data_tensor;
  torch::Tensor _response_tensor;
};

}

#endif
