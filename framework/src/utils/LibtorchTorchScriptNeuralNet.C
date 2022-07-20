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
    _nn_loaded = true;
  }
  catch (const c10::Error & e)
  {
    mooseError("Error while loading torchscript file!");
  }
}

torch::Tensor
LibtorchTorchScriptNeuralNet::forward(torch::Tensor x)
{
  if (!_nn_loaded)
    mooseError("The neural network has not been loaded so it makes no sense to evaluate it. Call "
               "loadNeuralNetwork() before using this function!");

  return LibtorchNeuralNet<torch::jit::script::Module>::forward(x);
}

}

#endif
