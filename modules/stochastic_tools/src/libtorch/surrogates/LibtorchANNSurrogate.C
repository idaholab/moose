//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  auto input_mean_accessor = input_mean.accessor<Real, 2>();
  auto input_std_accessor = input_std.accessor<Real, 2>();

  // unsigned long int i_mean = (unsigned long int)torch::size(input_mean, 0);
  // unsigned long int i_std = (unsigned long int)torch::size(input_std, 0);
  mooseAssert((unsigned long int)torch::size(input_mean, 0) == converted_input.size() &&
                  (unsigned long int)torch::size(input_std, 0) == converted_input.size(),
              "The input standardizer's dimensions should be the same as the input dimension!");

  for (auto input_i : index_range(converted_input))
    converted_input[input_i] =
        (x[input_i] - input_mean_accessor[input_i][0]) / input_std_accessor[input_i][0];

  torch::Tensor x_tf =
      torch::tensor(torch::ArrayRef<Real>(converted_input.data(), converted_input.size()))
          .to(at::kDouble);

  const auto & output_mean = _output_standardizer.getMean();
  const auto & output_std = _output_standardizer.getStdDev();

  auto output_mean_accessor = output_mean.accessor<Real, 2>();
  auto output_std_accessor = output_std.accessor<Real, 2>();

  mooseAssert(output_mean.sizes()[0] == 1 && output_std.sizes()[0] == 1,
              "The output standardizer's dimensions should be 1!");

  // Compute prediction
  val = _nn->forward(x_tf).item<double>();
  val = val * output_std_accessor[0][0] + output_mean_accessor[0][0];

  return val;
}

#endif
