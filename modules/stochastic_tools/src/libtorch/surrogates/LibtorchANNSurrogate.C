//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchANNSurrogate.h"

registerMooseObject("StochasticToolsApp", LibtorchANNSurrogate);

InputParameters
LibtorchANNSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Surrogate that evaluates a feedforward artificial neural net. ");
  return params;
}

LibtorchANNSurrogate::LibtorchANNSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _nn(getModelData<std::shared_ptr<Moose::LibtorchArtificialNeuralNet>>("nn"))
{
  // We check if MOOSE is compiled with torch, if not this throws an error
  StochasticToolsApp::requiresTorch(*this);
}

Real
LibtorchANNSurrogate::evaluate(const std::vector<Real> & x) const
{
  Real val(0.0);

  // Check whether input point has same dimensionality as training data
  mooseAssert(_nn->numInputs() == x.size(),
              "Input point does not match dimensionality of training data.");

  torch::Tensor x_tf = torch::tensor(torch::ArrayRef<Real>(x.data(), x.size())).to(at::kDouble);

  // Compute prediction
  val = _nn->forward(x_tf).item<double>();

  return val;
}

#endif
