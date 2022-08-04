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

#include "LibtorchNeuralNet.h"
#include "DataIO.h"
#include "MultiMooseEnum.h"

namespace Moose
{

// A class that describes a torch-script-based neural network.
class LibtorchTorchScriptNeuralNet : public LibtorchNeuralNet<torch::jit::script::Module>
{
public:
  /**
   * Construct using a filename which contains the source code in torchscript format
   * @param filename The number of output neurons
   */
  LibtorchTorchScriptNeuralNet(const std::string & filename);

  /// Return the name of the neural network
  const std::string & filename() const { return _filename; }

  /// Construct the neural network
  void loadNeuralNetwork(const std::string & filename);

  /**
   * Overriding the forward substitution function for the neural network, unfortunately
   * this cannot be const since it creates a graph in the background
   * @param x Input tensor for the evaluation
   */
  virtual torch::Tensor forward(torch::Tensor x) override;

protected:
  /// Name of the torchscript containing the neural network
  const std::string _filename;

  /// Switch to indicate if the neural network has been loaded or not
  bool _nn_loaded;
};

}

#endif
