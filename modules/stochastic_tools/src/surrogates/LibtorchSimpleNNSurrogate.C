//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibtorchSimpleNNSurrogate.h"

registerMooseObject("StochasticToolsApp", LibtorchSimpleNNSurrogate);

InputParameters
LibtorchSimpleNNSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Surrogate that evaluates a simple feedforward neural net. "
                             "See [LibtorchSimpleNNTrainer.md]");
  return params;
}

LibtorchSimpleNNSurrogate::LibtorchSimpleNNSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _no_hidden_layers(getModelData<unsigned int>("no_hidden_layers")),
    _no_neurons_per_layer(getModelData<std::vector<unsigned int>>("no_neurons_per_layer"))
#ifdef TORCH_ENABLED
    ,
    _nn(getModelData<std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet>>("nn"))
#endif
{
}

Real
LibtorchSimpleNNSurrogate::evaluate(const std::vector<Real> &
#ifdef TORCH_ENABLED
                                        x
#endif
) const
{
  Real val(0.0);

#ifdef TORCH_ENABLED

  // Check whether input point has same dimensionality as training data
  mooseAssert(_nn->noInputs() == x.size(),
              "Input point does not match dimensionality of training data.");

  torch::Tensor x_tf = torch::tensor(torch::ArrayRef<Real>(x.data(), x.size())).to(at::kDouble);

  // Compute prediction
  val = _nn->forward(x_tf).item<double>();
#endif

  return val;
}
