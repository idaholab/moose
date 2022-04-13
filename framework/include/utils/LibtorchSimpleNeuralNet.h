//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef TORCH_ENABLED

#include "LibtorchNeuralNetBase.h"

#include "DataIO.h"

namespace StochasticTools
{

// A class that describes a simple feed-forward neural net.
class LibtorchSimpleNeuralNet : public LibtorchNeuralNetBase
{
public:
  /// Construct using input parameters
  LibtorchSimpleNeuralNet(const std::string name,
                          const unsigned int num_inputs,
                          const std::vector<unsigned int> num_neurons_per_layer,
                          const unsigned int num_outputs);

  /**
   * Add layers to the neural network
   * @param layer_name The name of the layer to be added
   * @param parameters A map of parameter names and the corresponding values which
   *                   describe the neural net layer architecture
   */
  virtual void addLayer(const std::string layer_name,
                        const std::unordered_map<std::string, unsigned int> parameters) override;

  // Overriding the forward sustitution function
  torch::Tensor forward(torch::Tensor x);

  /// Return the name of the neural network
  const std::string & name() const { return _name; }
  /// Return the number of neurons on the input layer
  unsigned int numInputs() { return _num_inputs; }
  /// Return the number of hidden layers
  unsigned int numHiddenLayers() { return _num_hidden_layers; }
  /// Return the hidden layer architecture
  const std::vector<unsigned int> & numNeuronsPerLayer() const { return _num_neurons_per_layer; }
  /// Return the number of neurons on the output layer
  unsigned int numOutputs() { return _num_outputs; }
  /// Construct the neural network
  void constructNeuralNetwork();

protected:
  const std::string _name;

  // Submodules that hold linear operations and the corresponding
  // weights and biases (y = W * x + b)
  std::vector<torch::nn::Linear> _weights;

  const unsigned int _num_inputs;

  const std::vector<unsigned int> _num_neurons_per_layer;

  const unsigned int _num_hidden_layers;

  const unsigned int _num_outputs;
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
