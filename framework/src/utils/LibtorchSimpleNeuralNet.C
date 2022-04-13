//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef TORCH_ENABLED
#include "LibtorchSimpleNeuralNet.h"
#include "MooseError.h"

namespace StochasticTools
{

LibtorchSimpleNeuralNet::LibtorchSimpleNeuralNet(
    const std::string name,
    const unsigned int num_inputs,
    const std::vector<unsigned int> num_neurons_per_layer,
    const unsigned int num_outputs)
  : _name(name),
    _num_inputs(num_inputs),
    _num_neurons_per_layer(num_neurons_per_layer),
    _num_hidden_layers(num_neurons_per_layer.size()),
    _num_outputs(num_outputs)
{
  constructNeuralNetwork();
}

void
LibtorchSimpleNeuralNet::constructNeuralNetwork()
{
  // Adding hidden layers
  unsigned int inp_neurons = _num_inputs;
  for (unsigned int i = 0; i < _num_hidden_layers; ++i)
  {
    std::unordered_map<std::string, unsigned int> parameters = {
        {"inp_neurons", inp_neurons}, {"out_neurons", _num_neurons_per_layer[i]}};
    addLayer("hidden_layer_" + std::to_string(i + 1), parameters);

    // Necessary to retain double precision (and error-free runs)
    _weights[i]->to(at::kDouble);
    inp_neurons = _num_neurons_per_layer[i];
  }
  // Adding output layer
  std::unordered_map<std::string, unsigned int> parameters = {{"inp_neurons", inp_neurons},
                                                              {"out_neurons", _num_outputs}};
  addLayer("output_layer_", parameters);
  _weights.back()->to(at::kDouble);
}

torch::Tensor
LibtorchSimpleNeuralNet::forward(torch::Tensor x)
{
  for (unsigned int i = 0; i < _weights.size() - 1; ++i)
    x = torch::relu(_weights[i]->forward(x));

  x = _weights[_weights.size() - 1]->forward(x);

  return x.reshape({x.size(0)});
}

void
LibtorchSimpleNeuralNet::addLayer(std::string layer_name,
                                  std::unordered_map<std::string, unsigned int> parameters)
{
  auto it = parameters.find("inp_neurons");
  if (it == parameters.end())
    ::mooseError(
        "Number of input neurons not found during the construction of LibtorchSimpleNeuralNet!");
  unsigned int inp_neurons = it->second;

  it = parameters.find("out_neurons");
  if (it == parameters.end())
    ::mooseError(
        "Number of output neurons not found during the construction of LibtorchSimpleNeuralNet!");
  unsigned int out_neurons = it->second;

  _weights.push_back(register_module(layer_name, torch::nn::Linear(inp_neurons, out_neurons)));
}

}

template <>
void
dataStore<StochasticTools::LibtorchSimpleNeuralNet>(
    std::ostream & stream,
    std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
    void * context)
{
  std::string n(nn->name());
  dataStore(stream, n, context);

  unsigned int ni(nn->numInputs());
  dataStore(stream, ni, context);

  unsigned int nhl(nn->numHiddenLayers());
  dataStore(stream, nhl, context);

  std::vector<unsigned int> nnpl(nn->numNeuronsPerLayer());
  dataStore(stream, nnpl, context);

  unsigned int no(nn->numOutputs());
  dataStore(stream, no, context);

  torch::save(nn, nn->name());
}

template <>
void
dataLoad<StochasticTools::LibtorchSimpleNeuralNet>(
    std::istream & stream,
    std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
    void * context)
{
  std::string name;
  dataLoad(stream, name, context);

  unsigned int num_inputs;
  dataLoad(stream, num_inputs, context);

  unsigned int num_hidden_layers;
  dataLoad(stream, num_hidden_layers, context);

  std::vector<unsigned int> num_neurons_per_layer;
  num_neurons_per_layer.resize(num_hidden_layers);
  dataLoad(stream, num_neurons_per_layer, context);

  unsigned int num_outputs;
  dataLoad(stream, num_outputs, context);

  nn = std::make_shared<StochasticTools::LibtorchSimpleNeuralNet>(
      name, num_inputs, num_neurons_per_layer, num_outputs);

  torch::load(nn, name);
}

#endif
