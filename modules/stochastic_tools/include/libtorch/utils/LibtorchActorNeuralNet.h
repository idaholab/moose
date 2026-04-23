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
   * Construct using input parameters
   * @param name Name of the neural network
   * @param num_inputs The number of input neurons/parameters
   * @param num_neurons_per_layer Number of neurons per hidden layer
   * @param num_outputs The number of output neurons
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
   * Copy construct an artificial neural network
   * @param nn The neural network which needs to be copied
   */
  LibtorchActorNeuralNet(const Moose::LibtorchActorNeuralNet & nn,
                         const bool build_on_construct = true);

  /**
   * Overriding the forward substitution function for the neural network, unfortunately
   * this cannot be const since it creates a graph in the background
   * @param x Input tensor for the evaluation
   */
  virtual torch::Tensor forward(const torch::Tensor & x) override;

  virtual torch::Tensor evaluate(torch::Tensor & input, bool sampled);

  virtual torch::Tensor sample();

  /// Construct the neural network
  virtual void constructNeuralNetwork() override;

  const LibtorchActionDistribution & actionDistribution() const { return *_action_distribution; }
  LibtorchActionDistribution & actionDistribution() { return *_action_distribution; }

  const LibtorchGaussianActionDistribution * gaussianActionDistributionPtr() const;
  LibtorchGaussianActionDistribution * gaussianActionDistributionPtr();
  const LibtorchGaussianActionDistribution & gaussianActionDistribution() const;
  LibtorchGaussianActionDistribution & gaussianActionDistribution();

  const LibtorchBetaActionDistribution * betaActionDistributionPtr() const;
  LibtorchBetaActionDistribution * betaActionDistributionPtr();
  const LibtorchBetaActionDistribution & betaActionDistribution() const;
  LibtorchBetaActionDistribution & betaActionDistribution();

  bool stateIndependentStd() const { return _state_independent_std; }
  const std::vector<Real> & minValues() const { return _minimum_values; }
  const std::vector<Real> & maxValues() const { return _maximum_values; }

  void resetDistributionParams(torch::Tensor input);

  torch::Tensor logProbability(const torch::Tensor & other);

  torch::Tensor entropy();

  virtual void initializeNeuralNetwork() override;

protected:
  const std::vector<Real> _minimum_values;
  const std::vector<Real> _maximum_values;
  const bool _state_independent_std;
  std::shared_ptr<LibtorchActionDistribution> _action_distribution;
};

void to_json(nlohmann::json & json, const Moose::LibtorchActorNeuralNet * const & network);

void loadLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn,
                                     const std::string & filename);

bool isLegacyLibtorchActorArchive(const std::string & filename);

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

// This is needed because the reporter which is used to ouput the neural net parameters to JSON
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
