//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED
#include "LibtorchNeuralNet.h"

namespace Moose
{
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
