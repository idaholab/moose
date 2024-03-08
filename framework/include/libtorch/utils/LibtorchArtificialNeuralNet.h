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
#include "MooseError.h"
#include "DataIO.h"
#include "MultiMooseEnum.h"
#include "nlohmann/json.h"

namespace Moose
{

// A class that describes a simple feed-forward neural net.
class LibtorchArtificialNeuralNet : public torch::nn::Module, public LibtorchNeuralNetBase
{
public:
  /**
   * Construct using input parameters
   * @param name Name of the neural network
   * @param num_inputs The number of input neurons/parameters
   * @param num_neurons_per_layer Number of neurons per hidden layer
   * @param num_outputs The number of output neurons
   */
  LibtorchArtificialNeuralNet(const std::string name,
                              const unsigned int num_inputs,
                              const unsigned int num_outputs,
                              const std::vector<unsigned int> & num_neurons_per_layer,
                              const std::vector<std::string> & activation_function = {"relu"},
                              const torch::DeviceType device_type = torch::kCPU,
                              const torch::ScalarType scalar_type = torch::kDouble);

  /**
   * Copy construct an artificial neural network
   * @param nn The neural network which needs to be copied
   */
  LibtorchArtificialNeuralNet(const Moose::LibtorchArtificialNeuralNet & nn);

  /**
   * Add layers to the neural network
   * @param layer_name The name of the layer to be added
   * @param parameters A map of parameter names and the corresponding values which
   *                   describe the neural net layer architecture
   */
  virtual void addLayer(const std::string & layer_name,
                        const std::unordered_map<std::string, unsigned int> & parameters);

  /**
   * Overriding the forward substitution function for the neural network, unfortunately
   * this cannot be const since it creates a graph in the background
   * @param x Input tensor for the evaluation
   */
  virtual torch::Tensor forward(torch::Tensor & x) override;

  /// Return the name of the neural network
  const std::string & name() const { return _name; }
  /// Return the number of neurons on the input layer
  unsigned int numInputs() const { return _num_inputs; }
  /// Return the number of neurons on the output layer
  unsigned int numOutputs() const { return _num_outputs; }
  /// Return the number of hidden layers
  unsigned int numHiddenLayers() const { return _num_neurons_per_layer.size(); }
  /// Return the hidden layer architecture
  const std::vector<unsigned int> & numNeuronsPerLayer() const { return _num_neurons_per_layer; }
  /// Return the multi enum containing the activation functions
  const MultiMooseEnum & activationFunctions() const { return _activation_function; }
  /// Return the device which is used by this neural network
  torch::DeviceType deviceType() const { return _device_type; }
  /// Return the data type which is used by this neural network
  torch::ScalarType dataType() const { return _data_type; }
  /// Construct the neural network
  void constructNeuralNetwork();

  /// Store the network architecture in a json file (for debugging, visualization)
  void store(nlohmann::json & json) const;

protected:
  /// Name of the neural network
  const std::string _name;
  /// Submodules that hold linear operations and the corresponding
  /// weights and biases (y = W * x + b)
  std::vector<torch::nn::Linear> _weights;
  // Number of neurons on the input layer
  const unsigned int _num_inputs;
  /// Number of neurons on the output layer
  const unsigned int _num_outputs;
  /// Hidden layer architecture
  const std::vector<unsigned int> _num_neurons_per_layer;
  /// Activation functions (either one for all hidden layers or one for every layer
  /// separately)
  MultiMooseEnum _activation_function;
  /// The device type used for this neural network
  const torch::DeviceType _device_type;
  /// The data type used in this neural network
  const torch::ScalarType _data_type;
};

void to_json(nlohmann::json & json, const Moose::LibtorchArtificialNeuralNet * const & network);

}

template <>
void dataStore<Moose::LibtorchArtificialNeuralNet>(
    std::ostream & stream,
    std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn,
    void * context);

template <>
void dataLoad<Moose::LibtorchArtificialNeuralNet>(
    std::istream & stream,
    std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn,
    void * context);

// This is needed because the reporter which is used to ouput the neural net parameters to JSON
// requires a dataStore/dataLoad. However, these functions will be empty due to the fact that
// we are only interested in the JSON output and we don't want to output everything
template <>
void dataStore<Moose::LibtorchArtificialNeuralNet const>(
    std::ostream & stream, Moose::LibtorchArtificialNeuralNet const *& nn, void * context);

template <>
void dataLoad<Moose::LibtorchArtificialNeuralNet const>(
    std::istream & stream, Moose::LibtorchArtificialNeuralNet const *& nn, void * context);

#endif
