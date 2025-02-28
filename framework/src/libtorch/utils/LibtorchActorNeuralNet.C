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
LibtorchActorNeuralNet::initializeNeuralNetwork()
{
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    const auto & activation = _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    const Real gain = determineGain(activation);
    torch::nn::init::orthogonal_(_weights[i]->weight, gain);
    torch::nn::init::zeros_(_weights[i]->bias);
  }

  if (_minimum_values.size())
  {
    torch::nn::init::orthogonal_(_alpha_module[0]->weight);
    torch::nn::init::orthogonal_(_beta_module[0]->weight);
  }
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

  if (_minimum_values.size())
  {
    auto num_inps = _num_neurons_per_layer[numHiddenLayers()-1];
    _alpha_module.push_back(register_module("alpha", torch::nn::Linear(torch::nn::LinearOptions(num_inps, _num_outputs).bias(false))));
    _beta_module.push_back(register_module("beta", torch::nn::Linear(torch::nn::LinearOptions(num_inps, _num_outputs).bias(false))));
    _alpha_module[0]->to(_device_type, _data_type);
    _beta_module[0]->to(_device_type, _data_type);

    return;
  }

  torch::Tensor std_tensor = torch::eye(_std.size()).to(_data_type);
  for (unsigned int i = 0; i < _std.size(); ++i)
    std_tensor[i][i] = _std[i];

  _std_tensor = register_parameter("std", std_tensor);
}

torch::Tensor
LibtorchActorNeuralNet::entropy()
{
  if (_minimum_values.size())
  {
    return _log_norm - (_beta_tensor - 1.0) * torch::digamma(_beta_tensor)
           - (_alpha_tensor - 1.0) * torch::digamma(_alpha_tensor)
           + (_alpha_beta_tensor - 2.0) * torch::digamma(_alpha_beta_tensor);
  }

  return 0.5*std::log(2*M_PI)+torch::log(_std_tensor)+0.5;
}

void
LibtorchActorNeuralNet::resetDistributionParams(torch::Tensor input)
{
  if (_minimum_values.size())
  {
    auto alpha = _alpha_module[0]->forward(input);
    _alpha_tensor = torch::log(torch::exp(alpha) + 1.0) + 1.0;
    // std::cout << "setting alpha tensor to " << _alpha_tensor << std::endl;
    auto beta = _beta_module[0]->forward(input);
    _beta_tensor = torch::log(torch::exp(beta) + 1.0) + 1.0;
    // std::cout << "setting beta tensor to " << _beta_tensor << std::endl;

    _alpha_beta_tensor = torch::clamp_min(_alpha_tensor + _beta_tensor, 1e-8);
    _mean = _alpha_tensor/_alpha_beta_tensor;

    // std::cout << "setting mean to " << _mean << std::endl;

    _log_norm = at::lgamma(_alpha_tensor) + at::lgamma(_beta_tensor) - at::lgamma(_alpha_beta_tensor);

    return;
  }

  _mean = input;
}

torch::Tensor
LibtorchActorNeuralNet::forward(torch::Tensor & x)
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

  return _min_tensor + (_max_tensor - _min_tensor)*_mean;
}

torch::Tensor
LibtorchActorNeuralNet::sample()
{
  if (_minimum_values.size())
  {
    auto alpha_sample = at::_standard_gamma(_alpha_tensor);
    auto beta_sample = at::_standard_gamma(_beta_tensor);

    auto sampled = alpha_sample / (alpha_sample + beta_sample);

    // std::cout << "sampled " << sampled << std::endl;

    return _min_tensor + (_max_tensor - _min_tensor)*sampled;
  }

  return at::normal(_mean, _std_tensor);
}

torch::Tensor
LibtorchActorNeuralNet::logProbability(const torch::Tensor & action)
{
  // Logarithmic probability of taken action, given the current distribution.
  if (_minimum_values.size())
  {
    // std::cout << "input action " << action << std::endl;
    // std::cout << "mintensor " << _min_tensor << std::endl;
    // std::cout << "bewfore clamp " << (action - _min_tensor) / (_max_tensor - _min_tensor) <<std::endl;

    auto normalized = torch::clamp_max((action - _min_tensor) / (_max_tensor - _min_tensor), 1.0-1e-8);

    // std::cout << "normalized " << normalized << std::endl;
    // std::cout << "beta tensor " << _beta_tensor << std::endl;
    // std::cout << "_alpha_tensor " << _alpha_tensor << std::endl;
    // std::cout << "_lognorm " << _log_norm << std::endl;
    return (_beta_tensor - 1.0) * torch::log(torch::clamp_min(normalized, 1e-8)) + (_alpha_tensor - 1.0) * torch::log1p(-normalized) - _log_norm;
  }

  torch::Tensor var = torch::matmul(_std_tensor, _std_tensor);
  return -((action - _mean) * (action - _mean)) / (2.0 * var) - 0.5*torch::log(var) -
         0.5*std::log(2.0 * M_PI);
}

}


#endif
