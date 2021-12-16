//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <torch/torch.h>

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

class BasicNNTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  BasicNNTrainer(const InputParameters & parameters);

  virtual void preTrain() override;

  virtual void train() override;

  virtual void postTrain() override;

protected:
#ifdef ENABLE_PT

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

  // A structure that describes a simple feed-forward neural net.
  struct MyNet : torch::nn::Module
  {
  public:
    // Contructor building the neural net, not the coversions from float to double
    // due to the default type in pytorch is float and we use double in MOOSE.
    MyNet(unsigned int no_inputs,
          unsigned int no_hidden_layers,
          std::vector<unsigned int> no_neurons_per_layer,
          unsigned int no_outputs)
    {
      unsigned int inp_layers = no_inputs;
      for (unsigned int i = 0; i < no_hidden_layers; ++i)
      {
        _weights.push_back(register_module("HL" + std::to_string(i + 1),
                                           torch::nn::Linear(inp_layers, no_neurons_per_layer[i])));
        _weights[i]->to(at::kDouble);
        inp_layers = no_neurons_per_layer[i];
      }
      _weights.push_back(register_module("OL", torch::nn::Linear(inp_layers, no_outputs)));
      _weights.back()->to(at::kDouble);
    }

    // Overriding the forward substitution function
    torch::Tensor forward(torch::Tensor x)
    {
      for (unsigned int i = 0; i < _weights.size() - 1; ++i)
        x = torch::relu(_weights[i]->forward(x));

      x = _weights[_weights.size() - 1]->forward(x);

      return x.reshape({x.size(0)});
    }

  protected:
    // Submodules that hold linear operations and the corresponding
    // weights and biases (y = W * x + b)
    std::vector<torch::nn::Linear> _weights;
  };
#endif

private:
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
