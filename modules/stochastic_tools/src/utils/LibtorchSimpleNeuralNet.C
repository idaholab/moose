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

LibtorchSimpleNeuralNet::LibtorchSimpleNeuralNet(std::string name,
                                                 unsigned int no_inputs,
                                                 unsigned int no_hidden_layers,
                                                 std::vector<unsigned int> no_neurons_per_layer,
                                                 unsigned int no_outputs)
  : _name(name),
    _no_inputs(no_inputs),
    _no_hidden_layers(no_hidden_layers),
    _no_neurons_per_layer(no_neurons_per_layer),
    _no_outputs(no_outputs)
{
  constructNeuralNetwork();
}

void
LibtorchSimpleNeuralNet::constructNeuralNetwork()
{
  // Adding hidden layers
  unsigned int inp_neurons = _no_inputs;
  for (unsigned int i = 0; i < _no_hidden_layers; ++i)
  {
    std::unordered_map<std::string, unsigned int> parameters = {
        {"inp_neurons", inp_neurons}, {"out_neurons", _no_neurons_per_layer[i]}};
    addLayer("HL" + std::to_string(i + 1), parameters);

    // Necessary to retain double precision (and error-free runs)
    _weights[i]->to(at::kDouble);
    inp_neurons = _no_neurons_per_layer[i];
  }
  // Adding output layer
  std::unordered_map<std::string, unsigned int> parameters = {{"inp_neurons", inp_neurons},
                                                              {"out_neurons", _no_outputs}};
  addLayer("OL", parameters);
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

#endif
