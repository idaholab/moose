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
 * Reusable continuous-action distribution interface for actor policies.
 */
class LibtorchActionDistribution : public torch::nn::Module
{
public:
  /**
   * Build an action-distribution module for an actor network.
   * @param name Module name used for registration and serialization.
   * @param num_inputs Number of actor features feeding the distribution.
   * @param num_outputs Number of action dimensions produced by the distribution.
   * @param device_type Torch device used by the module.
   * @param scalar_type Torch scalar type used by the module.
   * @param output_scaling_factors Optional per-action scaling applied in physical units.
   */
  LibtorchActionDistribution(const std::string & name,
                             unsigned int num_inputs,
                             unsigned int num_outputs,
                             torch::DeviceType device_type = torch::kCPU,
                             torch::ScalarType scalar_type = torch::kDouble,
                             const std::vector<Real> & output_scaling_factors = {});

  /// Initialize the trainable distribution parameters.
  virtual void initialize(c10::optional<at::Generator> generator = c10::nullopt) = 0;

  /**
   * Refresh cached distribution parameters from the latest actor features.
   * @param input Feature tensor coming from the actor body.
   */
  virtual void reset(const torch::Tensor & input) = 0;

  /**
   * Draw a stochastic action sample in physical units.
   * @return Sampled action tensor.
   */
  virtual torch::Tensor sample(c10::optional<at::Generator> generator = c10::nullopt) const = 0;

  /**
   * Return the deterministic action used for evaluation.
   * @return Deterministic action tensor.
   */
  virtual torch::Tensor deterministicAction() const = 0;

  /**
   * Evaluate the log-probability of an action under the current distribution.
   * @param action Action tensor in physical units.
   * @return Log-probability tensor for the action.
   */
  virtual torch::Tensor logProbability(const torch::Tensor & action) const = 0;

  /**
   * Compute the entropy of the current distribution.
   * @return Entropy tensor.
   */
  virtual torch::Tensor entropy() const = 0;

  /**
   * Tell callers whether the distribution enforces explicit action bounds.
   * @return True for bounded distributions, false for unbounded ones.
   */
  virtual bool isBounded() const = 0;

  /// Sync cached scaling metadata from the registered buffers after loading state.
  void synchronizeScalingFactorsFromBuffer();

protected:
  torch::Tensor prepareFeatures(const torch::Tensor & input) const;
  torch::Tensor prepareAction(const torch::Tensor & action) const;
  const torch::Tensor & actionScaleTensor() const { return _action_scale_tensor; }

  const std::string _name;
  const unsigned int _num_inputs;
  const unsigned int _num_outputs;
  const torch::DeviceType _device_type;
  const torch::ScalarType _data_type;
  std::vector<Real> _output_scaling_factors;

  torch::Tensor _action_scale_tensor;
};

/**
 * Gaussian action distribution for unbounded action spaces.
 */
class LibtorchGaussianActionDistribution : public LibtorchActionDistribution
{
public:
  /**
   * Build the Gaussian action distribution used for unbounded controls.
   * @param name Module name used for registration and serialization.
   * @param num_inputs Number of actor features feeding the distribution.
   * @param num_outputs Number of action dimensions produced by the distribution.
   * @param device_type Torch device used by the module.
   * @param scalar_type Torch scalar type used by the module.
   * @param build_on_construct Whether to build the torch modules right away.
   * @param output_scaling_factors Optional per-action scaling applied in physical units.
   * @param state_independent_std Whether the learned std should ignore the current state.
   */
  LibtorchGaussianActionDistribution(const std::string & name,
                                     unsigned int num_inputs,
                                     unsigned int num_outputs,
                                     torch::DeviceType device_type = torch::kCPU,
                                     torch::ScalarType scalar_type = torch::kDouble,
                                     bool build_on_construct = true,
                                     const std::vector<Real> & output_scaling_factors = {},
                                     bool state_independent_std = true);

  virtual void initialize(c10::optional<at::Generator> generator = c10::nullopt) override;

  virtual void reset(const torch::Tensor & input) override;

  virtual torch::Tensor
  sample(c10::optional<at::Generator> generator = c10::nullopt) const override;

  virtual torch::Tensor deterministicAction() const override;

  virtual torch::Tensor logProbability(const torch::Tensor & action) const override;

  virtual torch::Tensor entropy() const override;

  virtual bool isBounded() const override { return false; }

  bool stateIndependentStd() const { return _state_independent_std; }
  torch::nn::Linear & meanModule() { return _mean_module; }
  const torch::nn::Linear & meanModule() const { return _mean_module; }
  torch::nn::Linear & stdModule() { return _std_module; }
  const torch::nn::Linear & stdModule() const { return _std_module; }
  const torch::Tensor & stdTensor() const { return _std_tensor; }

private:
  void constructDistribution();

  const bool _state_independent_std;
  torch::nn::Linear _mean_module{nullptr};
  torch::nn::Linear _std_module{nullptr};
  torch::Tensor _mean;
  torch::Tensor _std_tensor;
  torch::Tensor _log_std_tensor;
};

/**
 * Beta action distribution for bounded action spaces.
 */
class LibtorchBetaActionDistribution : public LibtorchActionDistribution
{
public:
  /**
   * Build the Beta action distribution used for bounded controls.
   * @param name Module name used for registration and serialization.
   * @param num_inputs Number of actor features feeding the distribution.
   * @param num_outputs Number of action dimensions produced by the distribution.
   * @param minimum_values Lower action bounds in physical units.
   * @param maximum_values Upper action bounds in physical units.
   * @param device_type Torch device used by the module.
   * @param scalar_type Torch scalar type used by the module.
   * @param build_on_construct Whether to build the torch modules right away.
   * @param output_scaling_factors Optional extra per-action scaling in physical units.
   */
  LibtorchBetaActionDistribution(const std::string & name,
                                 unsigned int num_inputs,
                                 unsigned int num_outputs,
                                 const std::vector<Real> & minimum_values,
                                 const std::vector<Real> & maximum_values,
                                 torch::DeviceType device_type = torch::kCPU,
                                 torch::ScalarType scalar_type = torch::kDouble,
                                 bool build_on_construct = true,
                                 const std::vector<Real> & output_scaling_factors = {});

  virtual void initialize(c10::optional<at::Generator> generator = c10::nullopt) override;

  virtual void reset(const torch::Tensor & input) override;

  virtual torch::Tensor
  sample(c10::optional<at::Generator> generator = c10::nullopt) const override;

  virtual torch::Tensor deterministicAction() const override;

  virtual torch::Tensor logProbability(const torch::Tensor & action) const override;

  virtual torch::Tensor entropy() const override;

  virtual bool isBounded() const override { return true; }

  torch::nn::Linear & alphaModule() { return _alpha_module; }
  const torch::nn::Linear & alphaModule() const { return _alpha_module; }
  torch::nn::Linear & betaModule() { return _beta_module; }
  const torch::nn::Linear & betaModule() const { return _beta_module; }
  const torch::Tensor & alphaTensor() const { return _alpha_tensor; }
  const torch::Tensor & betaTensor() const { return _beta_tensor; }

private:
  void constructDistribution();

  const std::vector<Real> _minimum_values;
  const std::vector<Real> _maximum_values;

  torch::nn::Linear _alpha_module{nullptr};
  torch::nn::Linear _beta_module{nullptr};
  torch::Tensor _min_tensor;
  torch::Tensor _max_tensor;
  torch::Tensor _alpha_tensor;
  torch::Tensor _beta_tensor;
  torch::Tensor _alpha_beta_tensor;
  torch::Tensor _log_norm;
  torch::Tensor _mean;
};

} // namespace Moose

#endif
