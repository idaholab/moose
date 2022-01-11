//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LibtorchNeuralNetBase.h"

#ifdef TORCH_ENABLED

namespace StochasticTools
{

// A class that describes a simple feed-forward neural net.
class LibtorchSimpleNeuralNet : public LibtorchNeuralNetBase
{
public:
  // Generic constructor
  LibtorchSimpleNeuralNet() {}

  // Constructing using input parameters
  LibtorchSimpleNeuralNet(unsigned int no_inputs,
                          unsigned int no_hidden_layers,
                          std::vector<unsigned int> no_neurons_per_layer,
                          unsigned int no_outputs);
  // Virtual destructor
  virtual ~LibtorchSimpleNeuralNet() {}

  // Overriding the function from NeuralNetBase
  virtual void addLayer(std::string layer_name,
                        std::unordered_map<std::string, unsigned int> parameters) override;

  // Overriding the forward sustitution function
  torch::Tensor forward(torch::Tensor x);

protected:
  // Submodules that hold linear operations and the corresponding
  // weights and biases (y = W * x + b)
  std::vector<torch::nn::Linear> _weights;
};

}

#endif
