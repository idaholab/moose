//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef TORCH_ENABLED
#include <torch/torch.h>
#include "LibtorchSimpleNeuralNet.h"
#endif

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

class LibtorchSimpleNNTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  LibtorchSimpleNNTrainer(const InputParameters & parameters);

  virtual void preTrain() override;

  virtual void train() override;

  virtual void postTrain() override;

protected:
#ifdef TORCH_ENABLED

  // A custom strcuture which is used to organize data foor the training of
  // torch-based neural nets.
  struct MyData : torch::data::datasets::Dataset<MyData>
  {
  public:
    MyData(torch::Tensor dt, torch::Tensor rt) : _data_tensor(dt), _response_tensor(rt) {}
    torch::data::Example<> get(size_t index) override
    {
      return {_data_tensor.slice(1, index, index + 1), _response_tensor[index]};
    }

    torch::optional<size_t> size() const override { return _response_tensor.sizes()[0]; }

  private:
    torch::Tensor _data_tensor;
    torch::Tensor _response_tensor;
  };

#endif

private:
  /// pointer to the neural net object (initialized as null)
  std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> _nn;

  /// Data from the current sampler row
  const std::vector<Real> & _sampler_row;

  /// Map containing sample points and the results
  std::vector<std::vector<Real>> & _sample_points;

  /// Predictor values from reporters
  std::vector<const Real *> _pvals;

  /// Columns from sampler for predictors
  std::vector<unsigned int> _pcols;

  /// Response value
  const Real & _response;

  /// Number of batches we want to prepare
  unsigned int _no_batches;

  /// Number of epochs for the training
  unsigned int _no_epocs;

  /// Number of hidden layers in the neural net
  unsigned int _no_hidden_layers;

  /// Number of neurons within the hidden layers (the length of this vector
  /// should be the same as _no_hidden_layers)
  std::vector<unsigned int> _no_neurons_per_layer;

  /// The learning rate for the optimization algorithm
  Real _learning_rate;
};
