//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include <torch/script.h>
#include "LibtorchNeuralNetBase.h"
#include "DataIO.h"
#include "MultiMooseEnum.h"

namespace Moose
{

// A class that describes a torch-script-based neural network.
class LibtorchTorchScriptNeuralNet : public torch::jit::script::Module, public LibtorchNeuralNetBase
{
public:

  /**
   * Construct using a filename which contains the source code in torchscript format
   */
  LibtorchTorchScriptNeuralNet();

  /**
   * Construct using a filename which contains the source code in torchscript format
   * @param filename The name of the file that contains the neural net
   */
  LibtorchTorchScriptNeuralNet(const std::string & filename);

  /// Construct the neural network
  void loadNeuralNetwork(const std::string & filename);

  /**
   * Overriding the forward substitution function for the neural network, unfortunately
   * this cannot be const since it creates a graph in the background
   * @param x Input tensor for the evaluation
   */
  virtual torch::Tensor forward(torch::Tensor & x) override;
};

}

#endif
