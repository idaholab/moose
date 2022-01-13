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

#include "SurrogateModel.h"

class LibtorchSimpleNNSurrogate : public SurrogateModel
{
public:
  static InputParameters validParams();
  LibtorchSimpleNNSurrogate(const InputParameters & parameters);

  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;

protected:
  /// Number of hidden layers in the neural net
  const unsigned int & _no_hidden_layers;

  /// Number of neurons within the hidden layers (the length of this vector
  /// should be the same as _no_hidden_layers)
  const std::vector<unsigned int> & _no_neurons_per_layer;

#ifdef TORCH_ENABLED
  /// Pointer to the neural net object (initialized as null)
  const std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & _nn;
#endif
};
