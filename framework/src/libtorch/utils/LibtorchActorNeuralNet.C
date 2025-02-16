//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchActorNeuralNet.h"
#include "MooseError.h"
#include "LibtorchUtils.h"

namespace Moose
{

LibtorchActorNeuralNet::LibtorchActorNeuralNet(
    const std::string name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const std::vector<unsigned int> & num_neurons_per_layer,
    const std::vector<Real> & std,
    const std::vector<std::string> & activation_function,
    const std::vector<Real> & minimum_values,
    const std::vector<Real> & maximum_values,
    const torch::DeviceType device_type,
    const torch::ScalarType data_type,
    const bool build_on_construct)
  :
  LibtorchArtificialNeuralNet(name, num_inputs, num_outputs, num_neurons_per_layer,
                              activation_function, minimum_values, maximum_values,
                              device_type, data_type,
                              false),
    _std(std)
{
  if (build_on_construct)
    constructNeuralNetwork();
}

LibtorchActorNeuralNet::LibtorchActorNeuralNet(
    const Moose::LibtorchActorNeuralNet & nn,
    const bool build_on_construct)
  :  LibtorchArtificialNeuralNet(dynamic_cast<const LibtorchArtificialNeuralNet &>(nn), false),
    _std(nn.std())
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
LibtorchActorNeuralNet::constructNeuralNetwork()
{
  LibtorchArtificialNeuralNet::constructNeuralNetwork();

  torch::Tensor std_tensor = torch::eye(_std.size()).to(_data_type);
  for (unsigned int i = 0; i < _std.size(); ++i)
    std_tensor[i][i] = _std[i];

  _std_tensor = register_parameter("std", std_tensor);
}

torch::Tensor
LibtorchActorNeuralNet::entropy()
{
  return 0.5*std::log(2*M_PI)+torch::log(_std_tensor)+0.5;
}

torch::Tensor
LibtorchActorNeuralNet::forward(torch::Tensor & x)
{
  torch::Tensor output(x);
  if (_data_type != output.scalar_type())
    output.to(_data_type);
  if (_device_type != output.device().type())
    output.to(_device_type);

  for (unsigned int i = 0; i < _weights.size() - 1; ++i)
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
  }

  if (_minimum_values.size())
  {
    output = torch::sigmoid(_weights[_weights.size() - 1]->forward(output));
    torch::Tensor scale = torch::sub(_max_tensor, _min_tensor).to(_data_type);
    output = torch::mul(output, scale);
    output = output + _min_tensor;
  }
  else
  {
    output = _weights[_weights.size() - 1]->forward(output);
  }

  _mean = output;
  auto action = at::normal(output, _std_tensor);
  _log_probability = computeLogProbability(action, output);

  output = torch::clamp(_min_tensor, _max_tensor, action);

  return output;
}

torch::Tensor
LibtorchActorNeuralNet::computeLogProbability(const torch::Tensor & action,
                                              const torch::Tensor & signal)
{
  // Logarithmic probability of taken action, given the current distribution.
  torch::Tensor var = torch::matmul(_std_tensor, _std_tensor);

  return -((action - signal) * (action - signal)) / (2.0 * var) - 0.5*torch::log(var) -
         0.5*std::log(2.0 * M_PI);
}

}


#endif
