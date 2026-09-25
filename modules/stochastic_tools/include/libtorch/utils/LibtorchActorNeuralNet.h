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
#include "LibtorchActionDistribution.h"
#include "LibtorchArtificialNeuralNet.h"

namespace Moose
{

/**
 * Feed-forward actor network coupled to a Gaussian or Beta action distribution.
 */
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

  virtual torch::Tensor forward(const torch::Tensor & x) override;

  /**
   * Evaluate the actor and either sample from it or use its deterministic action.
   * @param input Input tensor for the evaluation.
   * @param sampled Whether to draw a stochastic sample.
   * @param generator Optional random-number generator used for sampling.
   */
  virtual torch::Tensor evaluate(torch::Tensor & input,
                                 bool sampled,
                                 c10::optional<at::Generator> generator = c10::nullopt);

  /**
   * Sample an action from the already-reset distribution.
   * @param generator Optional random-number generator used for sampling.
   */
  virtual torch::Tensor sample(c10::optional<at::Generator> generator = c10::nullopt);

  virtual void constructNeuralNetwork() override;

  /// Return the active action distribution as the common base type.
  const LibtorchActionDistribution & actionDistribution() const { return *_action_distribution; }
  /// Return the active action distribution as the common base type.
  LibtorchActionDistribution & actionDistribution() { return *_action_distribution; }

  /// Return the Gaussian action distribution pointer, or nullptr for bounded actors.
  const LibtorchGaussianActionDistribution * gaussianActionDistributionPtr() const;
  /// Return the Gaussian action distribution pointer, or nullptr for bounded actors.
  LibtorchGaussianActionDistribution * gaussianActionDistributionPtr();
  /// Return the Gaussian action distribution reference. Errors if the actor is bounded.
  const LibtorchGaussianActionDistribution & gaussianActionDistribution() const;
  /// Return the Gaussian action distribution reference. Errors if the actor is bounded.
  LibtorchGaussianActionDistribution & gaussianActionDistribution();

  /// Return the Beta action distribution pointer, or nullptr for Gaussian actors.
  const LibtorchBetaActionDistribution * betaActionDistributionPtr() const;
  /// Return the Beta action distribution pointer, or nullptr for Gaussian actors.
  LibtorchBetaActionDistribution * betaActionDistributionPtr();
  /// Return the Beta action distribution reference. Errors if the actor is unbounded.
  const LibtorchBetaActionDistribution & betaActionDistribution() const;
  /// Return the Beta action distribution reference. Errors if the actor is unbounded.
  LibtorchBetaActionDistribution & betaActionDistribution();

  /// Return whether the Gaussian std ignores the current actor features.
  bool stateIndependentStd() const { return _state_independent_std; }
  /// Return the configured lower action bounds.
  const std::vector<Real> & minValues() const { return _minimum_values; }
  /// Return the configured upper action bounds.
  const std::vector<Real> & maxValues() const { return _maximum_values; }

  /**
   * Refresh the cached distribution parameters from a fresh input tensor.
   * @param input Input tensor used to update the distribution.
   */
  void resetDistributionParams(torch::Tensor input);

  /**
   * Evaluate the log-probability of an action under the current actor state.
   * @param other Action tensor in physical units.
   */
  torch::Tensor logProbability(const torch::Tensor & other);

  /// Compute the entropy of the current action distribution.
  torch::Tensor entropy();

  virtual void
  initializeNeuralNetwork(c10::optional<at::Generator> generator = c10::nullopt) override;

protected:
  /// Lower action bounds used by bounded actor distributions.
  const std::vector<Real> _minimum_values;
  /// Upper action bounds used by bounded actor distributions.
  const std::vector<Real> _maximum_values;
  /// Whether the Gaussian std ignores the current actor features.
  const bool _state_independent_std;
  /// Action-distribution module attached to the actor output.
  std::shared_ptr<LibtorchActionDistribution> _action_distribution;
};

/**
 * Dump an actor network into JSON for reporter output and debugging.
 * @param json JSON object that receives the serialized state.
 * @param network Actor network pointer to serialize.
 */
void to_json(nlohmann::json & json, const Moose::LibtorchActorNeuralNet * const & network);

/**
 * Load an actor checkpoint written as a native libtorch state archive.
 * @param nn Actor network that receives the loaded state.
 * @param filename Checkpoint file to read.
 */
void loadLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn,
                                     const std::string & filename);

}

/**
 * Serialize the actor-network metadata needed for restart.
 * @param stream Stream that receives the serialized data.
 * @param nn Actor network shared pointer to serialize.
 * @param context Serialization context passed through MOOSE data I/O.
 */
template <>
void dataStore<Moose::LibtorchActorNeuralNet>(std::ostream & stream,
                                              std::shared_ptr<Moose::LibtorchActorNeuralNet> & nn,
                                              void * context);

/**
 * Deserialize the actor-network metadata needed for restart.
 * @param stream Stream that provides the serialized data.
 * @param nn Actor network shared pointer to populate.
 * @param context Serialization context passed through MOOSE data I/O.
 */
template <>
void dataLoad<Moose::LibtorchActorNeuralNet>(std::istream & stream,
                                             std::shared_ptr<Moose::LibtorchActorNeuralNet> & nn,
                                             void * context);

// This is needed because the reporter which is used to output the neural net parameters to JSON
// requires a dataStore/dataLoad. However, these functions will be empty due to the fact that
// we are only interested in the JSON output and we don't want to output everything
/**
 * Placeholder serializer for reporter-only actor pointers.
 * @param stream Stream that would receive the serialized data.
 * @param nn Reporter actor pointer.
 * @param context Serialization context passed through MOOSE data I/O.
 */
template <>
void dataStore<Moose::LibtorchActorNeuralNet const>(std::ostream & stream,
                                                    Moose::LibtorchActorNeuralNet const *& nn,
                                                    void * context);

/**
 * Placeholder deserializer for reporter-only actor pointers.
 * @param stream Stream that would provide the serialized data.
 * @param nn Reporter actor pointer.
 * @param context Serialization context passed through MOOSE data I/O.
 */
template <>
void dataLoad<Moose::LibtorchActorNeuralNet const>(std::istream & stream,
                                                   Moose::LibtorchActorNeuralNet const *& nn,
                                                   void * context);

#endif
