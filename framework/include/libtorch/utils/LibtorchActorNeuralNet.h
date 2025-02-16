//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include <torch/script.h>
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
                         const std::vector<Real> & std,
                         const std::vector<std::string> & activation_function = {"relu"},
                         const std::vector<Real> & minimum_values = {},
                         const std::vector<Real> & maximum_values = {},
                         const torch::DeviceType device_type = torch::kCPU,
                         const torch::ScalarType scalar_type = torch::kDouble,
                         const bool build_on_construct = true);

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
  virtual torch::Tensor forward(torch::Tensor & x) override;

  /// Construct the neural network
  virtual void constructNeuralNetwork() override;

  const std::vector<Real> & std() const {return _std;};

  const torch::Tensor & stdTensor() const {return  _std_tensor;}

  torch::Tensor computeLogProbability(const torch::Tensor & action,
                                                const torch::Tensor & signal);

  torch::Tensor logProbability() {return _log_probability;}
  torch::Tensor logProbability(torch::Tensor other) {return computeLogProbability(_mean, other);}

  torch::Tensor entropy();

protected:
  const std::vector<Real> & _std;

  torch::Tensor _std_tensor;

  torch::Tensor _mean;
  torch::Tensor _log_probability;
};

void to_json(nlohmann::json & json, const Moose::LibtorchActorNeuralNet * const & network);

}

template <>
void dataStore<Moose::LibtorchActorNeuralNet>(
    std::ostream & stream,
    std::shared_ptr<Moose::LibtorchActorNeuralNet> & nn,
    void * context);

template <>
void dataLoad<Moose::LibtorchActorNeuralNet>(
    std::istream & stream,
    std::shared_ptr<Moose::LibtorchActorNeuralNet> & nn,
    void * context);

// This is needed because the reporter which is used to ouput the neural net parameters to JSON
// requires a dataStore/dataLoad. However, these functions will be empty due to the fact that
// we are only interested in the JSON output and we don't want to output everything
template <>
void dataStore<Moose::LibtorchActorNeuralNet const>(
    std::ostream & stream, Moose::LibtorchActorNeuralNet const *& nn, void * context);

template <>
void dataLoad<Moose::LibtorchActorNeuralNet const>(
    std::istream & stream, Moose::LibtorchActorNeuralNet const *& nn, void * context);

#endif
