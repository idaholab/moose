//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>

#include "MooseTypes.h"

#include <string>
#include <vector>

namespace Moose
{

/**
 * Reusable continuous-action distribution head for actor policies.
 *
 * Unbounded actions use a Gaussian parameterization. If both minimum and maximum values are
 * provided, the head switches to a bounded Beta parameterization.
 */
class LibtorchActionDistributionHead : public torch::nn::Module
{
public:
  LibtorchActionDistributionHead(const std::string & name,
                                 unsigned int num_inputs,
                                 unsigned int num_outputs,
                                 const std::vector<Real> & minimum_values = {},
                                 const std::vector<Real> & maximum_values = {},
                                 torch::DeviceType device_type = torch::kCPU,
                                 torch::ScalarType scalar_type = torch::kDouble,
                                 bool build_on_construct = true,
                                 const std::vector<Real> & output_scaling_factors = {},
                                 bool state_independent_std = true);

  LibtorchActionDistributionHead(const LibtorchActionDistributionHead & head,
                                 bool build_on_construct = true);

  void constructHead();

  void initialize();

  void synchronizeScalingFactorsFromBuffer();

  void reset(const torch::Tensor & input);

  torch::Tensor sample() const;

  torch::Tensor deterministicAction() const;

  torch::Tensor logProbability(const torch::Tensor & action) const;

  torch::Tensor entropy() const;

  bool isBounded() const { return !_minimum_values.empty(); }
  bool stateIndependentStd() const { return _state_independent_std; }

  torch::nn::Linear & primaryModule() { return _primary_parameter_module; }
  const torch::nn::Linear & primaryModule() const { return _primary_parameter_module; }

  torch::nn::Linear & secondaryModule() { return _secondary_parameter_module; }
  const torch::nn::Linear & secondaryModule() const { return _secondary_parameter_module; }

  const torch::Tensor & stdTensor() const { return _std_tensor; }
  const torch::Tensor & alphaTensor() const { return _alpha_tensor; }
  const torch::Tensor & betaTensor() const { return _beta_tensor; }

private:
  const std::string _name;
  const unsigned int _num_inputs;
  const unsigned int _num_outputs;
  const std::vector<Real> _minimum_values;
  const std::vector<Real> _maximum_values;
  const torch::DeviceType _device_type;
  const torch::ScalarType _data_type;
  const bool _state_independent_std;
  std::vector<Real> _output_scaling_factors;

  torch::nn::Linear _primary_parameter_module{nullptr};
  torch::nn::Linear _secondary_parameter_module{nullptr};

  torch::Tensor _action_scale_tensor;
  torch::Tensor _min_tensor;
  torch::Tensor _max_tensor;
  torch::Tensor _alpha_tensor;
  torch::Tensor _beta_tensor;
  torch::Tensor _alpha_beta_tensor;
  torch::Tensor _log_norm;
  torch::Tensor _mean_tensor;
  torch::Tensor _std_tensor;
  torch::Tensor _log_std_tensor;
  torch::Tensor _mean;
};

} // namespace Moose

#endif
