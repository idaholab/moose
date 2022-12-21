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
   * @param filename The number of output neurons
   */
  LibtorchTorchScriptNeuralNet(const std::string & filename);

  /// Return the name of the neural network
  const std::string & filename() const { return _filename; }

  /**
   * Overriding the forward substitution function for the neural network, unfortunately
   * this cannot be const since it creates a graph in the background
   * @param x Input tensor for the evaluation
   */
  virtual torch::Tensor forward(torch::Tensor & x) override;

protected:
  /// Name of the torchscript containing the neural network
  const std::string _filename;

private:
  /// Construct the neural network
  void loadNeuralNetwork(const std::string & filename);
};

}

#endif
