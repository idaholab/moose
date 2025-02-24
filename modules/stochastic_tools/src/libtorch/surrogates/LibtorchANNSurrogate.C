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
    _nn(getModelData<std::shared_ptr<Moose::LibtorchArtificialNeuralNet>>("nn")),
    _input_standardizer(getModelData<StochasticTools::Standardizer>("input_standardizer")),
    _output_standardizer(getModelData<StochasticTools::Standardizer>("output_standardizer"))
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

  std::vector<Real> converted_input(x.size(), 0);
  const auto & input_mean = _input_standardizer.getMean();
  const auto & input_std = _input_standardizer.getStdDev();

  mooseAssert(input_mean.size() == converted_input.size() &&
                  input_std.size() == converted_input.size(),
              "The input standardizer's dimensions should be the same as the input dimension!");

  for (auto input_i : index_range(converted_input))
    converted_input[input_i] = (x[input_i] - input_mean[input_i]) / input_std[input_i];

  torch::Tensor x_tf =
      torch::tensor(torch::ArrayRef<Real>(converted_input.data(), converted_input.size()))
          .to(at::kDouble);

  const auto & output_mean = _output_standardizer.getMean();
  const auto & output_std = _output_standardizer.getStdDev();

  mooseAssert(output_mean.size() == 1 && output_std.size() == 1,
              "The output standardizer's dimensions should be 1!");

  // Compute prediction
  val = _nn->forward(x_tf).item<double>();
  val = val * output_std[0] + output_mean[0];

  return val;
}

#endif
