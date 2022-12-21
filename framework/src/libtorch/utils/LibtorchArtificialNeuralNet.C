//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchArtificialNeuralNet.h"
#include "MooseError.h"

namespace Moose
{

LibtorchArtificialNeuralNet::LibtorchArtificialNeuralNet(
    const std::string name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const std::vector<unsigned int> & num_neurons_per_layer,
    const std::vector<std::string> & activation_function)
  : _name(name),
    _num_inputs(num_inputs),
    _num_outputs(num_outputs),
    _num_neurons_per_layer(num_neurons_per_layer),
    _activation_function(MultiMooseEnum("relu sigmoid elu gelu linear", "relu"))
{
  _activation_function = activation_function;

  // Check if the number of activation functions matches the number of hidden layers
  if ((_activation_function.size() != 1) &&
      (_activation_function.size() != _num_neurons_per_layer.size()))
    mooseError("The number of activation functions should be either one or the same as the number "
               "of hidden layers");
  constructNeuralNetwork();
}

LibtorchArtificialNeuralNet::LibtorchArtificialNeuralNet(
    const Moose::LibtorchArtificialNeuralNet & nn)
  : torch::nn::Module(),
    _name(nn.name()),
    _num_inputs(nn.numInputs()),
    _num_outputs(nn.numOutputs()),
    _num_neurons_per_layer(nn.numNeuronsPerLayer()),
    _activation_function(nn.activationFunctions())
{

  // We construct the NN architecture
  constructNeuralNetwork();
  // We fill it up with the current parameter values
  const auto & from_params = nn.named_parameters();
  auto to_params = this->named_parameters();
  for (unsigned int param_i : make_range(from_params.size()))
    to_params[param_i].value().data() = from_params[param_i].value().data().clone();
}

void
LibtorchArtificialNeuralNet::constructNeuralNetwork()
{
  // Adding hidden layers
  unsigned int inp_neurons = _num_inputs;
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
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
LibtorchArtificialNeuralNet::forward(torch::Tensor & x)
{
  torch::Tensor output(x);
  for (unsigned int i = 0; i < _weights.size() - 1; ++i)
  {
    std::string activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    if (activation == "relu")
      output = torch::relu(_weights[i]->forward(output));
    else if (activation == "sigmoid")
      output = torch::sigmoid(_weights[i]->forward(output));
    else if (activation == "elu")
      output = torch::elu(_weights[i]->forward(output));
    else if (activation == "gelu")
      output = torch::gelu(_weights[i]->forward(output));
    else if (activation == "linear")
      output = _weights[i]->forward(output);
  }

  output = _weights[_weights.size() - 1]->forward(output);

  return output;
}

void
LibtorchArtificialNeuralNet::addLayer(
    const std::string & layer_name,
    const std::unordered_map<std::string, unsigned int> & parameters)
{
  auto it = parameters.find("inp_neurons");
  if (it == parameters.end())
    ::mooseError("Number of input neurons not found during the construction of "
                 "LibtorchArtificialNeuralNet!");
  unsigned int inp_neurons = it->second;

  it = parameters.find("out_neurons");
  if (it == parameters.end())
    ::mooseError("Number of output neurons not found during the construction of "
                 "LibtorchArtificialNeuralNet!");
  unsigned int out_neurons = it->second;

  _weights.push_back(register_module(layer_name, torch::nn::Linear(inp_neurons, out_neurons)));
}

void
LibtorchArtificialNeuralNet::store(nlohmann::json & json) const
{
  const auto & named_params = this->named_parameters();
  for (const auto & param_i : make_range(named_params.size()))
  {
    // We cast the parameters into a 1D vector
    json[named_params[param_i].key()] = std::vector<Real>(
        named_params[param_i].value().data_ptr<Real>(),
        named_params[param_i].value().data_ptr<Real>() + named_params[param_i].value().numel());
  }
}

void
to_json(nlohmann::json & json, const Moose::LibtorchArtificialNeuralNet * const & network)
{
  if (network)
    network->store(json);
}

}

template <>
void
dataStore<Moose::LibtorchArtificialNeuralNet>(
    std::ostream & stream, std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn, void * context)
{
  std::string n(nn->name());
  dataStore(stream, n, context);

  unsigned int ni(nn->numInputs());
  dataStore(stream, ni, context);

  unsigned int no(nn->numOutputs());
  dataStore(stream, no, context);

  unsigned int nhl(nn->numHiddenLayers());
  dataStore(stream, nhl, context);

  std::vector<unsigned int> nnpl(nn->numNeuronsPerLayer());
  dataStore(stream, nnpl, context);

  unsigned int afs(nn->activationFunctions().size());
  dataStore(stream, afs, context);

  std::vector<std::string> items(afs);
  for (unsigned int i = 0; i < afs; ++i)
    items[i] = nn->activationFunctions()[i];

  dataStore(stream, items, context);

  torch::save(nn, nn->name());
}

template <>
void
dataLoad<Moose::LibtorchArtificialNeuralNet>(
    std::istream & stream, std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn, void * context)
{
  std::string name;
  dataLoad(stream, name, context);

  unsigned int num_inputs;
  dataLoad(stream, num_inputs, context);

  unsigned int num_outputs;
  dataLoad(stream, num_outputs, context);

  unsigned int num_hidden_layers;
  dataLoad(stream, num_hidden_layers, context);

  std::vector<unsigned int> num_neurons_per_layer;
  num_neurons_per_layer.resize(num_hidden_layers);
  dataLoad(stream, num_neurons_per_layer, context);

  unsigned int num_activation_items;
  dataLoad(stream, num_activation_items, context);

  std::vector<std::string> activation_functions;
  activation_functions.resize(num_activation_items);
  dataLoad(stream, activation_functions, context);

  nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      name, num_inputs, num_outputs, num_neurons_per_layer, activation_functions);

  torch::load(nn, name);
}

template <>
void
dataStore<Moose::LibtorchArtificialNeuralNet const>(
    std::ostream & /*stream*/,
    Moose::LibtorchArtificialNeuralNet const *& /*nn*/,
    void * /*context*/)
{
}

template <>
void
dataLoad<Moose::LibtorchArtificialNeuralNet const>(
    std::istream & /*stream*/,
    Moose::LibtorchArtificialNeuralNet const *& /*nn*/,
    void * /*context*/)
{
}

#endif
