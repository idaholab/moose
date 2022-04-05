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

#include "DataIO.h"

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
                          unsigned int num_inputs,
                          unsigned int num_hidden_layers,
                          std::vector<unsigned int> num_neurons_per_layer,
                          unsigned int num_outputs);
  // Virtual destructor
  virtual ~LibtorchSimpleNeuralNet() {}

  // Overriding the function from NeuralNetBase
  virtual void addLayer(const std::string layer_name,
                        const std::unordered_map<std::string, unsigned int> parameters) override;

  // Overriding the forward sustitution function
  torch::Tensor forward(torch::Tensor x);

  const std::string & name() { return _name; }

  const unsigned int & numInputs() { return _num_inputs; }

  const unsigned int & numHiddenLayers() { return _num_hidden_layers; }

  const std::vector<unsigned int> & numNeuronsPerLayer() { return _num_neurons_per_layer; }

  const unsigned int & numOutputs() { return _num_outputs; }

  void setName(std::string name) { _name = name; }

  void setNumInputs(unsigned int num_inputs) { _num_inputs = num_inputs; }

  void setNumHiddenLayers(unsigned int num_hidden_layers)
  {
    _num_hidden_layers = num_hidden_layers;
  }

  void setNumNeuronsPerLayer(std::vector<unsigned int> num_neurons_per_layer)
  {
    _num_neurons_per_layer = num_neurons_per_layer;
  }

  void setNumOutputs(unsigned int num_outputs) { _num_outputs = num_outputs; }

  void constructNeuralNetwork();

protected:
  std::string _name;

  // Submodules that hold linear operations and the corresponding
  // weights and biases (y = W * x + b)
  std::vector<torch::nn::Linear> _weights;

  unsigned int _num_inputs;

  unsigned int _num_hidden_layers;

  std::vector<unsigned int> _num_neurons_per_layer;

  unsigned int _num_outputs;
};

}

template <>
void dataStore<StochasticTools::LibtorchSimpleNeuralNet>(
    std::ostream & stream,
    std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
    void * context);

template <>
void dataLoad<StochasticTools::LibtorchSimpleNeuralNet>(
    std::istream & stream,
    std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
    void * context);

#endif
