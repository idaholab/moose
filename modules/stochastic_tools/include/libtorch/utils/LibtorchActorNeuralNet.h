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
#include <torch/script.h>
#include "LibtorchActionDistribution.h"
#include "LibtorchArtificialNeuralNet.h"

namespace Moose
{

class LibtorchActorNeuralNet : public LibtorchArtificialNeuralNet
{
public:
  /**
   * Build an actor network with either a Gaussian or Beta action distribution.
   * @param name Name of the neural network module.
   * @param num_inputs Number of actor inputs.
   * @param num_outputs Number of action outputs.
   * @param num_neurons_per_layer Hidden-layer widths.
   * @param activation_function Hidden-layer activation names.
   * @param minimum_values Optional lower action bounds. Leave empty for Gaussian actions.
   * @param maximum_values Optional upper action bounds. Leave empty for Gaussian actions.
   * @param device_type Torch device used by the module.
   * @param scalar_type Torch scalar type used by the module.
   * @param build_on_construct Whether to build the torch modules right away.
   * @param input_shift_factors Optional affine input shifts.
   * @param input_scaling_factors Optional affine input scales.
   * @param output_scaling_factors Optional per-action scaling in physical units.
   * @param state_independent_std Whether the Gaussian std should ignore the current state.
   */
  LibtorchActorNeuralNet(const std::string name,
                         const unsigned int num_inputs,
                         const unsigned int num_outputs,
                         const std::vector<unsigned int> & num_neurons_per_layer,
                         const std::vector<std::string> & activation_function = {"relu"},
                         const std::vector<Real> & minimum_values = {},
                         const std::vector<Real> & maximum_values = {},
                         const torch::DeviceType device_type = torch::kCPU,
                         const torch::ScalarType scalar_type = torch::kDouble,
                         const bool build_on_construct = true,
                         const std::vector<Real> & input_shift_factors = {},
                         const std::vector<Real> & input_scaling_factors = {},
                         const std::vector<Real> & output_scaling_factors = {},
                         const bool state_independent_std = true);

  /**
   * Copy-construct an actor network.
   * @param nn Actor network to copy.
   * @param build_on_construct Whether to rebuild the module structure during the copy.
   */
  LibtorchActorNeuralNet(const Moose::LibtorchActorNeuralNet & nn,
                         const bool build_on_construct = true);

  /**
   * Run the actor forward pass and return a sampled action.
   * @param x Input tensor for the evaluation.
   * @return Action tensor produced by the actor.
   */
  virtual torch::Tensor forward(const torch::Tensor & x) override;

  /**
   * Evaluate the actor and either sample from it or use its deterministic action.
   * @param input Input tensor for the evaluation.
   * @param sampled Whether to draw a stochastic sample.
   * @return Action tensor produced by the actor.
   */
  virtual torch::Tensor evaluate(torch::Tensor & input, bool sampled);

  /**
   * Sample an action from the already-reset distribution.
   * @return Sampled action tensor.
   */
  virtual torch::Tensor sample();

  /// Build the hidden layers and the matching action-distribution module.
  virtual void constructNeuralNetwork() override;

  /// Return the active action distribution as the common base type.
  const LibtorchActionDistribution & actionDistribution() const { return *_action_distribution; }
  LibtorchActionDistribution & actionDistribution() { return *_action_distribution; }

  /// Return the Gaussian action distribution pointer, or nullptr for bounded actors.
  const LibtorchGaussianActionDistribution * gaussianActionDistributionPtr() const;
  LibtorchGaussianActionDistribution * gaussianActionDistributionPtr();
  /// Return the Gaussian action distribution reference. Errors if the actor is bounded.
  const LibtorchGaussianActionDistribution & gaussianActionDistribution() const;
  LibtorchGaussianActionDistribution & gaussianActionDistribution();

  /// Return the Beta action distribution pointer, or nullptr for Gaussian actors.
  const LibtorchBetaActionDistribution * betaActionDistributionPtr() const;
  LibtorchBetaActionDistribution * betaActionDistributionPtr();
  /// Return the Beta action distribution reference. Errors if the actor is unbounded.
  const LibtorchBetaActionDistribution & betaActionDistribution() const;
  LibtorchBetaActionDistribution & betaActionDistribution();

  bool stateIndependentStd() const { return _state_independent_std; }
  const std::vector<Real> & minValues() const { return _minimum_values; }
  const std::vector<Real> & maxValues() const { return _maximum_values; }

  /**
   * Refresh the cached distribution parameters from a fresh input tensor.
   * @param input Input tensor used to update the distribution.
   */
  void resetDistributionParams(torch::Tensor input);

  /**
   * Evaluate the log-probability of an action under the current actor state.
   * @param other Action tensor in physical units.
   * @return Log-probability tensor.
   */
  torch::Tensor logProbability(const torch::Tensor & other);

  /**
   * Compute the entropy of the current action distribution.
   * @return Entropy tensor.
   */
  torch::Tensor entropy();

  /// Initialize the hidden layers and action-distribution parameters.
  virtual void initializeNeuralNetwork() override;

protected:
  const std::vector<Real> _minimum_values;
  const std::vector<Real> _maximum_values;
  const bool _state_independent_std;
  std::shared_ptr<LibtorchActionDistribution> _action_distribution;
};

/**
 * Dump an actor network into JSON for reporter output and debugging.
 * @param json JSON object that receives the serialized state.
 * @param network Actor network pointer to serialize.
 */
void to_json(nlohmann::json & json, const Moose::LibtorchActorNeuralNet * const & network);

/**
 * Load an actor checkpoint written either as a state archive or TorchScript module.
 * @param nn Actor network that receives the loaded state.
 * @param filename Checkpoint file to read.
 */
void loadLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn,
                                     const std::string & filename);

/**
 * Check whether a checkpoint comes from the older serialized-parameter layout.
 * @param filename Checkpoint file to inspect.
 * @return True if the file matches the legacy actor format.
 */
bool isLegacyLibtorchActorArchive(const std::string & filename);

/**
 * Load a checkpoint that still uses the legacy actor serialization layout.
 * @param nn Actor network that receives the loaded state.
 * @param filename Checkpoint file to read.
 * @param action_standard_deviations Fallback std values for older Gaussian checkpoints.
 */
void loadLegacyLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn,
                                           const std::string & filename,
                                           const std::vector<Real> & action_standard_deviations);

}

template <>
void dataStore<Moose::LibtorchActorNeuralNet>(std::ostream & stream,
                                              std::shared_ptr<Moose::LibtorchActorNeuralNet> & nn,
                                              void * context);

template <>
void dataLoad<Moose::LibtorchActorNeuralNet>(std::istream & stream,
                                             std::shared_ptr<Moose::LibtorchActorNeuralNet> & nn,
                                             void * context);

// This is needed because the reporter which is used to output the neural net parameters to JSON
// requires a dataStore/dataLoad. However, these functions will be empty due to the fact that
// we are only interested in the JSON output and we don't want to output everything
template <>
void dataStore<Moose::LibtorchActorNeuralNet const>(std::ostream & stream,
                                                    Moose::LibtorchActorNeuralNet const *& nn,
                                                    void * context);

template <>
void dataLoad<Moose::LibtorchActorNeuralNet const>(std::istream & stream,
                                                   Moose::LibtorchActorNeuralNet const *& nn,
                                                   void * context);

#endif
