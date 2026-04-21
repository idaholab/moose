//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchActorNeuralNet.h"
#include "MooseError.h"

namespace Moose
{

LibtorchActorNeuralNet::LibtorchActorNeuralNet(
    const std::string name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const std::vector<unsigned int> & num_neurons_per_layer,
    const std::vector<std::string> & activation_function,
    const std::vector<Real> & minimum_values,
    const std::vector<Real> & maximum_values,
    const torch::DeviceType device_type,
    const torch::ScalarType data_type,
    const bool build_on_construct)
  : LibtorchArtificialNeuralNet(name,
                                num_inputs,
                                num_outputs,
                                num_neurons_per_layer,
                                activation_function,
                                minimum_values,
                                maximum_values,
                                device_type,
                                data_type,
                                false)
{
  if (build_on_construct)
    constructNeuralNetwork();
}

LibtorchActorNeuralNet::LibtorchActorNeuralNet(const Moose::LibtorchActorNeuralNet & nn,
                                               const bool build_on_construct)
  : LibtorchArtificialNeuralNet(dynamic_cast<const LibtorchArtificialNeuralNet &>(nn), false)
{
  // We construct the NN architecture
  if (build_on_construct)
  {
    constructNeuralNetwork();
    // We fill it up with the current parameter values
    const auto & from_params = nn.named_parameters();
    auto to_params = this->named_parameters();
    for (unsigned int param_i : make_range(from_params.size()))
      to_params[param_i].value().data() = from_params[param_i].value().data().clone();
  }
}

void
LibtorchActorNeuralNet::initializeNeuralNetwork()
{
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    const auto & activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    const Real gain = determineGain(activation);

    auto sizes = _weights[i]->weight.sizes();
    auto max_dim_size = *std::max_element(sizes.begin(), sizes.end());
    torch::nn::init::orthogonal_(_weights[i]->weight, gain / max_dim_size);
    torch::nn::init::zeros_(_weights[i]->bias);
  }

  _action_head->initialize();
}

void
LibtorchActorNeuralNet::constructNeuralNetwork()
{
  // Adding hidden layers
  unsigned int inp_neurons = _num_inputs;
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    std::unordered_map<std::string, unsigned int> parameters = {
        {"inp_neurons", inp_neurons}, {"out_neurons", _num_neurons_per_layer[i]}};
    addLayer("hidden_layer_" + std::to_string(i + 1), parameters);

    // Necessary to retain double precision (and error-free runs)
    _weights[i]->to(_device_type, _data_type);
    inp_neurons = _num_neurons_per_layer[i];
  }

  _action_head = std::make_shared<LibtorchActionDistributionHead>(
      "action_head",
      inp_neurons,
      _num_outputs,
      _minimum_values,
      _maximum_values,
      _device_type,
      _data_type);
  register_module("action_head", _action_head);
}

torch::Tensor
LibtorchActorNeuralNet::entropy()
{
  return _action_head->entropy();
}

void
LibtorchActorNeuralNet::resetDistributionParams(torch::Tensor input)
{
  _action_head->reset(input);
  _alpha_tensor = _action_head->alphaTensor();
  _beta_tensor = _action_head->betaTensor();
  _std_tensor = _action_head->stdTensor();
}

torch::Tensor
LibtorchActorNeuralNet::forward(const torch::Tensor & x)
{
  torch::Tensor output(x);
  if (_data_type != output.scalar_type())
    output.to(_data_type);
  if (_device_type != output.device().type())
    output.to(_device_type);

  for (unsigned int i = 0; i < _weights.size(); ++i)
  {
    std::string activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    if (activation == "relu")
      output = torch::relu(_weights[i]->forward(output));
    else if (activation == "sigmoid")
      output = torch::sigmoid(_weights[i]->forward(output));
    else if (activation == "tanh")
      output = torch::tanh(_weights[i]->forward(output));
    else if (activation == "elu")
      output = torch::elu(_weights[i]->forward(output));
    else if (activation == "gelu")
      output = torch::gelu(_weights[i]->forward(output));
    else if (activation == "linear")
      output = _weights[i]->forward(output);

    // std::cout << "midresult" << i << output << std::endl;
  }

  return output;
}

torch::Tensor
LibtorchActorNeuralNet::evaluate(torch::Tensor & x, bool sampled)
{
  torch::Tensor output(x);
  // std::cout << output << std::endl;
  if (_data_type != output.scalar_type())
    output.to(_data_type);
  if (_device_type != output.device().type())
    output.to(_device_type);

  // std::cout << "input" << output << std::endl;
  output = forward(output);

  // std::cout << "midresult" << output << std::endl;
  resetDistributionParams(output);

  if (sampled)
    return sample();

  return _action_head->deterministicAction();
}

torch::Tensor
LibtorchActorNeuralNet::sample()
{
  return _action_head->sample();
}

torch::Tensor
LibtorchActorNeuralNet::logProbability(const torch::Tensor & action)
{
  return _action_head->logProbability(action);
}

}

#endif
