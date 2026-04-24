//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchActionDistribution.h"

#include "LibtorchUtils.h"
#include "MooseError.h"

#include <cmath>
#include <limits>
#include "libmesh/utility.h"

namespace
{

/**
 * Fill in default action scaling and catch shape mistakes early.
 * @param factors User-provided scaling factors.
 * @param expected_size Number of action outputs expected by the distribution.
 * @return A fully populated scaling-factor vector.
 */
std::vector<Real>
normalizeActionScalingFactors(const std::vector<Real> & factors, const unsigned int expected_size)
{
  const auto normalized = factors.empty() ? std::vector<Real>(expected_size, 1.0) : factors;

  if (normalized.size() != expected_size)
    mooseError("The number of output_scaling_factors entries must match the number of action "
               "outputs.");

  for (const auto factor : normalized)
    if (std::abs(factor) == 0.0)
      mooseError("The output_scaling_factors entries must be non-zero.");

  return normalized;
}

} // namespace

namespace Moose
{

LibtorchActionDistribution::LibtorchActionDistribution(
    const std::string & name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const torch::DeviceType device_type,
    const torch::ScalarType data_type,
    const std::vector<Real> & output_scaling_factors)
  : _name(name),
    _num_inputs(num_inputs),
    _num_outputs(num_outputs),
    _device_type(device_type),
    _data_type(data_type),
    _output_scaling_factors(normalizeActionScalingFactors(output_scaling_factors, num_outputs))
{
  auto action_scale = _output_scaling_factors;
  LibtorchUtils::vectorToTensor(action_scale, _action_scale_tensor);
  _action_scale_tensor = register_buffer(
      "action_scale", _action_scale_tensor.transpose(0, 1).to(_data_type).to(_device_type));
}

void
LibtorchActionDistribution::synchronizeScalingFactorsFromBuffer()
{
  auto action_scale =
      _action_scale_tensor.detach().reshape({-1}).to(torch::kCPU).to(torch::kDouble);
  LibtorchUtils::tensorToVector(action_scale, _output_scaling_factors);
}

torch::Tensor
LibtorchActionDistribution::prepareFeatures(const torch::Tensor & input) const
{
  auto features = input;
  if (_data_type != features.scalar_type())
    features = features.to(_data_type);
  if (_device_type != features.device().type())
    features = features.to(_device_type);
  return features;
}

torch::Tensor
LibtorchActionDistribution::prepareAction(const torch::Tensor & action) const
{
  auto scaled_action = action;
  if (_data_type != scaled_action.scalar_type())
    scaled_action = scaled_action.to(_data_type);
  if (_device_type != scaled_action.device().type())
    scaled_action = scaled_action.to(_device_type);
  return scaled_action;
}

LibtorchGaussianActionDistribution::LibtorchGaussianActionDistribution(
    const std::string & name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const torch::DeviceType device_type,
    const torch::ScalarType data_type,
    const bool build_on_construct,
    const std::vector<Real> & output_scaling_factors,
    const bool state_independent_std)
  : LibtorchActionDistribution(
        name, num_inputs, num_outputs, device_type, data_type, output_scaling_factors),
    _state_independent_std(state_independent_std)
{
  if (build_on_construct)
    constructDistribution();
}

void
LibtorchGaussianActionDistribution::constructDistribution()
{
  _mean_module = register_module(
      "mean", torch::nn::Linear(torch::nn::LinearOptions(_num_inputs, _num_outputs).bias(true)));
  _std_module = register_module(
      "std", torch::nn::Linear(torch::nn::LinearOptions(_num_inputs, _num_outputs).bias(true)));

  _mean_module->to(_device_type, _data_type);
  _std_module->to(_device_type, _data_type);
}

void
LibtorchGaussianActionDistribution::initialize()
{
  torch::nn::init::orthogonal_(_mean_module->weight);
  torch::nn::init::zeros_(_mean_module->bias);

  if (_state_independent_std)
  {
    _std_module->weight.data().zero_();
    torch::nn::init::zeros_(_std_module->bias);
    return;
  }

  torch::nn::init::orthogonal_(_std_module->weight);
  torch::nn::init::zeros_(_std_module->bias);
}

void
LibtorchGaussianActionDistribution::reset(const torch::Tensor & input)
{
  const auto features = prepareFeatures(input);
  _mean = _mean_module->forward(features);

  if (_state_independent_std)
  {
    if (_mean.dim() <= 1)
      _log_std_tensor = _std_module->bias;
    else
      _log_std_tensor = _std_module->bias.view({1, -1}).expand(_mean.sizes());
  }
  else
    _log_std_tensor = _std_module->forward(features);

  _log_std_tensor = torch::clamp(_log_std_tensor, std::log(1e-12), -std::log(1e-12));
  _std_tensor = torch::exp(_log_std_tensor);
}

torch::Tensor
LibtorchGaussianActionDistribution::sample() const
{
  return at::normal(_mean, _std_tensor) * actionScaleTensor();
}

torch::Tensor
LibtorchGaussianActionDistribution::deterministicAction() const
{
  return _mean * actionScaleTensor();
}

torch::Tensor
LibtorchGaussianActionDistribution::logProbability(const torch::Tensor & action) const
{
  const auto scaled_action = prepareAction(action);
  const auto log_action_scale = torch::log(torch::abs(actionScaleTensor()));
  const auto unscaled_action = scaled_action / actionScaleTensor();

  constexpr Real pi = 3.14159265358979323846;
  const torch::Tensor var = _std_tensor * _std_tensor;
  return -((unscaled_action - _mean) * (unscaled_action - _mean)) / (2.0 * var) - _log_std_tensor -
         0.5 * std::log(2.0 * pi) - log_action_scale;
}

torch::Tensor
LibtorchGaussianActionDistribution::entropy() const
{
  const auto log_action_scale = torch::log(torch::abs(actionScaleTensor()));
  constexpr Real pi = 3.14159265358979323846;
  return 0.5 * std::log(2.0 * pi) + _log_std_tensor + 0.5 + log_action_scale;
}

LibtorchBetaActionDistribution::LibtorchBetaActionDistribution(
    const std::string & name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const std::vector<Real> & minimum_values,
    const std::vector<Real> & maximum_values,
    const torch::DeviceType device_type,
    const torch::ScalarType data_type,
    const bool build_on_construct,
    const std::vector<Real> & output_scaling_factors)
  : LibtorchActionDistribution(
        name, num_inputs, num_outputs, device_type, data_type, output_scaling_factors),
    _minimum_values(minimum_values),
    _maximum_values(maximum_values)
{
  if (_minimum_values.size() != _num_outputs || _maximum_values.size() != _num_outputs)
    mooseError("The number of minimum_values and maximum_values entries must match the number "
               "of action outputs.");

  for (const auto i : make_range(_minimum_values.size()))
    if (!(_maximum_values[i] > _minimum_values[i]))
      mooseError("maximum_values entries must be strictly greater than minimum_values entries.");

  auto min_value = _minimum_values;
  LibtorchUtils::vectorToTensor(min_value, _min_tensor);
  _min_tensor = _min_tensor.transpose(0, 1).to(_data_type).to(_device_type);
  auto max_value = _maximum_values;
  LibtorchUtils::vectorToTensor(max_value, _max_tensor);
  _max_tensor = _max_tensor.transpose(0, 1).to(_data_type).to(_device_type);

  if (build_on_construct)
    constructDistribution();
}

void
LibtorchBetaActionDistribution::constructDistribution()
{
  _alpha_module = register_module(
      "alpha", torch::nn::Linear(torch::nn::LinearOptions(_num_inputs, _num_outputs).bias(true)));
  _beta_module = register_module(
      "beta", torch::nn::Linear(torch::nn::LinearOptions(_num_inputs, _num_outputs).bias(true)));

  _alpha_module->to(_device_type, _data_type);
  _beta_module->to(_device_type, _data_type);
}

void
LibtorchBetaActionDistribution::initialize()
{
  torch::nn::init::orthogonal_(_alpha_module->weight);
  torch::nn::init::zeros_(_alpha_module->bias);

  torch::nn::init::orthogonal_(_beta_module->weight);
  torch::nn::init::zeros_(_beta_module->bias);
}

void
LibtorchBetaActionDistribution::reset(const torch::Tensor & input)
{
  const auto features = prepareFeatures(input);
  const auto alpha = _alpha_module->forward(features);
  _alpha_tensor = torch::log(torch::exp(alpha) + 1.0) + 1.0;
  const auto beta = _beta_module->forward(features);
  _beta_tensor = torch::log(torch::exp(beta) + 1.0) + 1.0;

  _alpha_beta_tensor = torch::clamp_min(_alpha_tensor + _beta_tensor, 1e-8);
  _mean = _alpha_tensor / _alpha_beta_tensor;
  _log_norm = at::lgamma(_alpha_tensor) + at::lgamma(_beta_tensor) - at::lgamma(_alpha_beta_tensor);
}

torch::Tensor
LibtorchBetaActionDistribution::sample() const
{
  const auto alpha_sample = at::_standard_gamma(_alpha_tensor);
  const auto beta_sample = at::_standard_gamma(_beta_tensor);
  const auto sampled = alpha_sample / (alpha_sample + beta_sample);
  return (_min_tensor + (_max_tensor - _min_tensor) * sampled) * actionScaleTensor();
}

torch::Tensor
LibtorchBetaActionDistribution::deterministicAction() const
{
  return (_min_tensor + (_max_tensor - _min_tensor) * _mean) * actionScaleTensor();
}

torch::Tensor
LibtorchBetaActionDistribution::logProbability(const torch::Tensor & action) const
{
  const auto scaled_action = prepareAction(action);
  const auto log_action_scale = torch::log(torch::abs(actionScaleTensor()));
  const auto unscaled_action = scaled_action / actionScaleTensor();
  const auto scale = torch::clamp_min(_max_tensor - _min_tensor, 1e-8);
  const auto normalized = (unscaled_action - _min_tensor) / scale;
  const auto clipped = torch::clamp(normalized, 1e-8, 1.0 - 1e-8);
  auto log_prob = (_alpha_tensor - 1.0) * torch::log(clipped) +
                  (_beta_tensor - 1.0) * torch::log1p(-clipped) - _log_norm - torch::log(scale) -
                  log_action_scale;

  const auto out_of_bounds = (normalized < 0.0) | (normalized > 1.0);
  if (out_of_bounds.any().item<bool>())
    log_prob = torch::where(out_of_bounds,
                            torch::full_like(log_prob, -std::numeric_limits<Real>::infinity()),
                            log_prob);

  return log_prob;
}

torch::Tensor
LibtorchBetaActionDistribution::entropy() const
{
  const auto log_action_scale = torch::log(torch::abs(actionScaleTensor()));
  const auto scale = torch::clamp_min(_max_tensor - _min_tensor, 1e-8);
  return _log_norm - (_beta_tensor - 1.0) * torch::digamma(_beta_tensor) -
         (_alpha_tensor - 1.0) * torch::digamma(_alpha_tensor) +
         (_alpha_beta_tensor - 2.0) * torch::digamma(_alpha_beta_tensor) + torch::log(scale) +
         log_action_scale;
}

} // namespace Moose

#endif
