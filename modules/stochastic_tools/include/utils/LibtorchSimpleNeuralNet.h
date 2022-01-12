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
  LibtorchSimpleNeuralNet(std::string name,
                          unsigned int no_inputs,
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

  std::string name() { return _name; }

  unsigned int noInputs() { return _no_inputs; }

  unsigned int noHiddenLayers() { return _no_hidden_layers; }

  std::vector<unsigned int> noNeuronsPerLayer() { return _no_neurons_per_layer; }

  unsigned int noOutputs() { return _no_outputs; }

  void setName(std::string name) { _name = name; }

  void setNoInputs(unsigned int no_inputs) { _no_inputs = no_inputs; }

  void setNoHiddenLayers(unsigned int no_hidden_layers) { _no_hidden_layers = no_hidden_layers; }

  void setNoNeuronsPerLayer(std::vector<unsigned int> no_neurons_per_layer)
  {
    _no_neurons_per_layer = no_neurons_per_layer;
  }

  void setNoOutputs(unsigned int no_outputs) { _no_outputs = no_outputs; }

  void constructNeuralNetwork();

protected:
  std::string _name;

  // Submodules that hold linear operations and the corresponding
  // weights and biases (y = W * x + b)
  std::vector<torch::nn::Linear> _weights;

  unsigned int _no_inputs;

  unsigned int _no_hidden_layers;

  std::vector<unsigned int> _no_neurons_per_layer;

  unsigned int _no_outputs;
};

}

#endif
