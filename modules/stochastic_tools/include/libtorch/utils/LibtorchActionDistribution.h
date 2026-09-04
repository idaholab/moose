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
   * @param generator Optional random-number generator used for sampling.
   */
  virtual torch::Tensor sample(c10::optional<at::Generator> generator = c10::nullopt) const = 0;

  /// Return the deterministic action used for evaluation.
  virtual torch::Tensor deterministicAction() const = 0;

  /**
   * Evaluate the log-probability of an action under the current distribution.
   * @param action Action tensor in physical units.
   */
  virtual torch::Tensor logProbability(const torch::Tensor & action) const = 0;

  /// Compute the entropy of the current distribution.
  virtual torch::Tensor entropy() const = 0;

  /// Tell callers whether the distribution enforces explicit action bounds.
  virtual bool isBounded() const = 0;

  /// Sync cached scaling metadata from the registered buffers after loading state.
  void synchronizeScalingFactorsFromBuffer();

protected:
  /**
   * Convert actor features to the configured device and scalar type.
   * @param input Raw actor feature tensor.
   */
  torch::Tensor prepareFeatures(const torch::Tensor & input) const;
  /**
   * Convert actions to the configured device and scalar type.
   * @param action Raw action tensor.
   */
  torch::Tensor prepareAction(const torch::Tensor & action) const;
  /// Return the registered tensor that stores per-action scaling factors.
  const torch::Tensor & actionScaleTensor() const { return _action_scale_tensor; }

  /// Module name used for registration and serialization.
  const std::string _name;
  /// Number of actor features feeding this distribution.
  const unsigned int _num_inputs;
  /// Number of action dimensions produced by this distribution.
  const unsigned int _num_outputs;
  /// Torch device used by this distribution.
  const torch::DeviceType _device_type;
  /// Torch scalar type used by this distribution.
  const torch::ScalarType _data_type;
  /// Cached per-action scaling factors applied in physical units.
  std::vector<Real> _output_scaling_factors;

  /// Registered libtorch buffer holding the per-action scaling factors.
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

  /// Return whether the Gaussian std ignores the current actor features.
  bool stateIndependentStd() const { return _state_independent_std; }
  /// Return the Gaussian mean head.
  torch::nn::Linear & meanModule() { return _mean_module; }
  /// Return the Gaussian mean head.
  const torch::nn::Linear & meanModule() const { return _mean_module; }
  /// Return the Gaussian std head.
  torch::nn::Linear & stdModule() { return _std_module; }
  /// Return the Gaussian std head.
  const torch::nn::Linear & stdModule() const { return _std_module; }
  /// Return the cached Gaussian standard deviation tensor.
  const torch::Tensor & stdTensor() const { return _std_tensor; }

private:
  /// Build and register the Gaussian distribution heads.
  void constructDistribution();

  /// Whether the Gaussian std ignores the current actor features.
  const bool _state_independent_std;
  /// Linear head that produces the Gaussian action mean.
  torch::nn::Linear _mean_module{nullptr};
  /// Linear head that produces the Gaussian log-std inputs or bias-only std state.
  torch::nn::Linear _std_module{nullptr};
  /// Cached Gaussian action mean from the latest reset.
  torch::Tensor _mean;
  /// Cached Gaussian action standard deviation from the latest reset.
  torch::Tensor _std_tensor;
  /// Cached Gaussian action log standard deviation from the latest reset.
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

  /// Return the Beta alpha head.
  torch::nn::Linear & alphaModule() { return _alpha_module; }
  /// Return the Beta alpha head.
  const torch::nn::Linear & alphaModule() const { return _alpha_module; }
  /// Return the Beta beta head.
  torch::nn::Linear & betaModule() { return _beta_module; }
  /// Return the Beta beta head.
  const torch::nn::Linear & betaModule() const { return _beta_module; }
  /// Return the cached Beta alpha tensor.
  const torch::Tensor & alphaTensor() const { return _alpha_tensor; }
  /// Return the cached Beta beta tensor.
  const torch::Tensor & betaTensor() const { return _beta_tensor; }

private:
  /// Build and register the Beta distribution heads.
  void constructDistribution();

  /// Lower action bounds in physical units.
  const std::vector<Real> _minimum_values;
  /// Upper action bounds in physical units.
  const std::vector<Real> _maximum_values;

  /// Linear head that produces the Beta alpha parameters.
  torch::nn::Linear _alpha_module{nullptr};
  /// Linear head that produces the Beta beta parameters.
  torch::nn::Linear _beta_module{nullptr};
  /// Tensor form of the lower action bounds.
  torch::Tensor _min_tensor;
  /// Tensor form of the upper action bounds.
  torch::Tensor _max_tensor;
  /// Cached Beta alpha parameters from the latest reset.
  torch::Tensor _alpha_tensor;
  /// Cached Beta beta parameters from the latest reset.
  torch::Tensor _beta_tensor;
  /// Cached sum of the alpha and beta parameters.
  torch::Tensor _alpha_beta_tensor;
  /// Cached log normalization factor for the latest Beta distribution state.
  torch::Tensor _log_norm;
  /// Cached normalized Beta mean from the latest reset.
  torch::Tensor _mean;
};

} // namespace Moose

#endif
