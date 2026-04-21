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
#include "LibtorchUtils.h"

#include <limits>

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

  if (_minimum_values.size())
  {
    auto sizes = _alpha_module[0]->weight.sizes();
    auto max_dim_size = *std::max_element(sizes.begin(), sizes.end());
    torch::nn::init::orthogonal_(_alpha_module[0]->weight, 1.0 / max_dim_size);
    torch::nn::init::orthogonal_(_beta_module[0]->weight, 1.0 / max_dim_size);
  }
  else
  {
    auto sizes = _mean_module[0]->weight.sizes();
    auto max_dim_size = *std::max_element(sizes.begin(), sizes.end());
    torch::nn::init::orthogonal_(_mean_module[0]->weight, 1.0 / max_dim_size);
    torch::nn::init::orthogonal_(_log_std_module[0]->weight, 1.0 / max_dim_size);
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

  auto num_inps = _num_neurons_per_layer[numHiddenLayers() - 1];
  if (_minimum_values.size())
  {
    _alpha_module.push_back(register_module(
        "alpha", torch::nn::Linear(torch::nn::LinearOptions(num_inps, _num_outputs).bias(false))));
    _beta_module.push_back(register_module(
        "beta", torch::nn::Linear(torch::nn::LinearOptions(num_inps, _num_outputs).bias(false))));
    _alpha_module[0]->to(_device_type, _data_type);
    _beta_module[0]->to(_device_type, _data_type);

    return;
  }

  _mean_module.push_back(register_module(
      "mean", torch::nn::Linear(torch::nn::LinearOptions(num_inps, _num_outputs).bias(false))));
  _log_std_module.push_back(register_module(
      "std", torch::nn::Linear(torch::nn::LinearOptions(num_inps, _num_outputs).bias(false))));
  _mean_module[0]->to(_device_type, _data_type);
  _log_std_module[0]->to(_device_type, _data_type);
}

torch::Tensor
LibtorchActorNeuralNet::entropy()
{
  if (_minimum_values.size())
  {
    const auto scale = torch::clamp_min(_max_tensor - _min_tensor, 1e-8);
    return _log_norm - (_beta_tensor - 1.0) * torch::digamma(_beta_tensor) -
           (_alpha_tensor - 1.0) * torch::digamma(_alpha_tensor) +
           (_alpha_beta_tensor - 2.0) * torch::digamma(_alpha_beta_tensor) + torch::log(scale);
  }

  return 0.5 * std::log(2 * M_PI) + _log_std_tensor + 0.5;
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
    _mean = _alpha_tensor / _alpha_beta_tensor;

    // std::cout << "setting mean to " << _mean << std::endl;

    _log_norm =
        at::lgamma(_alpha_tensor) + at::lgamma(_beta_tensor) - at::lgamma(_alpha_beta_tensor);

    return;
  }

  // # Flat mean and log standard deviation
  //       mean = self.mean.apply(x=x)
  //       log_stddev = self.log_stddev.apply(x=x)

  //       # Reshape mean and log stddev to action shape
  //       shape = (-1,) + self.shape
  //       mean = tf.reshape(tensor=mean, shape=shape)
  //       log_stddev = tf.reshape(tensor=log_stddev, shape=shape)

  //       # Clip log stddev for numerical stability
  //       log_eps = log(util.epsilon)  # epsilon < 1.0, hence negative
  //       log_stddev = tf.clip_by_value(t=log_stddev, clip_value_min=log_eps,
  //       clip_value_max=-log_eps)

  //       # Standard deviation
  //       stddev = tf.exp(x=log_stddev)

  //       return mean, stddev, log_stddev

  _mean = _mean_module[0]->forward(input);
  _log_std_tensor = _log_std_module[0]->forward(input);

  _log_std_tensor = torch::clamp(_log_std_tensor, std::log(1e-12), -std::log(1e-12));
  _std_tensor = torch::exp(_log_std_tensor);
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

  if (_minimum_values.size())
    return _min_tensor + (_max_tensor - _min_tensor) * _mean;

  return _mean;
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

    return _min_tensor + (_max_tensor - _min_tensor) * sampled;
  }

  return at::normal(_mean, _std_tensor);
}

torch::Tensor
LibtorchActorNeuralNet::logProbability(const torch::Tensor & action)
{
  // Logarithmic probability of taken action, given the current distribution.
  if (_minimum_values.size())
  {
    const auto scale = torch::clamp_min(_max_tensor - _min_tensor, 1e-8);
    const auto normalized = (action - _min_tensor) / scale;
    const auto clipped = torch::clamp(normalized, 1e-8, 1.0 - 1e-8);
    auto log_prob = (_alpha_tensor - 1.0) * torch::log(clipped) +
                    (_beta_tensor - 1.0) * torch::log1p(-clipped) - _log_norm - torch::log(scale);

    const auto out_of_bounds = (normalized < 0.0) | (normalized > 1.0);
    if (out_of_bounds.any().item<bool>())
      log_prob = torch::where(out_of_bounds,
                              torch::full_like(log_prob, -std::numeric_limits<Real>::infinity()),
                              log_prob);

    return log_prob;
  }

  torch::Tensor var = _std_tensor * _std_tensor;
  return -((action - _mean) * (action - _mean)) / (2.0 * var) - _log_std_tensor -
         0.5 * std::log(2.0 * M_PI);
}

}

#endif
