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
#include <torch/script.h>
#include "LibtorchNeuralNetBase.h"
#include "MooseError.h"

namespace Moose
{

/**
 * This class is templated to allow inheritance from
 * regular libtorch Modules and TorchScript Modules.
 */
template <typename T>
class LibtorchNeuralNet : public T, public LibtorchNeuralNetBase
{
public:
  // Virtual destructor
  virtual ~LibtorchNeuralNet() {}

  // Override the evaluate function
  virtual torch::Tensor forward(torch::Tensor x) override;
};

// Ex[plicitly instantiate the two different libtorch module branches
template class LibtorchNeuralNet<torch::nn::Module>;
template class LibtorchNeuralNet<torch::jit::script::Module>;

// The forward function for a self-defined module will de defined on a case-by-case basis
template <typename T>
torch::Tensor LibtorchNeuralNet<T>::forward(torch::Tensor /*x*/)
{
  ::mooseError("The evaluate function is not implemented for LibtorchNeuralNet! Override your "
               "function in the derived classes!");
}

// The forward function for a torch-script-based module will be always the one in this definition
template <>
torch::Tensor
LibtorchNeuralNet<torch::jit::script::Module>::forward(torch::Tensor x)
{
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(x);
  return torch::jit::script::Module::forward(inputs).toTensor();
}

}

#endif
