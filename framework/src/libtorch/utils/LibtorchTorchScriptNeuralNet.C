//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include <torch/torch.h>
#include "LibtorchTorchScriptNeuralNet.h"
#include "MooseError.h"

namespace Moose
{

LibtorchTorchScriptNeuralNet::LibtorchTorchScriptNeuralNet(const std::string & filename)
  : _filename(filename)
{
  loadNeuralNetwork(_filename);
}

void
LibtorchTorchScriptNeuralNet::loadNeuralNetwork(const std::string & filename)
{
  try
  {
    torch::jit::script::Module * base = this;
    *base = torch::jit::load(filename);
  }
  catch (const c10::Error & e)
  {
    mooseError("Error while loading torchscript file ", filename, "!\n", e.msg());
  }
}

torch::Tensor
LibtorchTorchScriptNeuralNet::forward(torch::Tensor & x)
{
  std::vector<torch::jit::IValue> inputs(1, x);
  return torch::jit::script::Module::forward(inputs).toTensor();
}

}

#endif
