//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include <torch/torch.h>
#include "TorchScriptModule.h"
#include "MooseError.h"

namespace Moose
{

TorchScriptModule::TorchScriptModule() {}

TorchScriptModule::TorchScriptModule(const std::string & filename) { loadNeuralNetwork(filename); }

void
TorchScriptModule::loadNeuralNetwork(const std::string & filename)
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
TorchScriptModule::forward(const torch::Tensor & x)
{
  std::vector<torch::jit::IValue> inputs(1, x);
  return torch::jit::script::Module::forward(inputs).toTensor();
}

}

#endif
