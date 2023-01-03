//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include <torch/torch.h>
#include "LibtorchTorchScriptNeuralNet.h"
#include "LibtorchTorchScriptNeuralNetTest.h"

registerMooseObject("MooseTestApp", LibtorchTorchScriptNeuralNetTest);

InputParameters
LibtorchTorchScriptNeuralNetTest::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addParam<std::string>(
      "filename", "my_net.pt", "The name of the file where the torch script is saved.");
  params.addClassDescription("Evaluates a neural network saved from python.");
  return params;
}

LibtorchTorchScriptNeuralNetTest::LibtorchTorchScriptNeuralNetTest(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _x_values(declareVector("x_values")),
    _y_values(declareVector("y_values")),
    _z_values(declareVector("z_values")),
    _nn_values(declareVector("nn_values"))
{
  // We read the neural net from a file which was prepared in python
  std::shared_ptr<Moose::LibtorchNeuralNetBase> nn =
      std::make_shared<Moose::LibtorchTorchScriptNeuralNet>(getParam<std::string>("filename"));

  // First, we evaluate the neural network at (0.0, 0.0, 0.0) and add it to the VPPs
  torch::Tensor input = at::zeros({1, 3}, at::kDouble);
  _x_values.push_back(0.0);
  _y_values.push_back(0.0);
  _z_values.push_back(0.0);

  torch::Tensor prediction = nn->forward(input);
  _nn_values.push_back(prediction.item<double>());

  // Now we evaluate the neural net at two other positions and add the values to the VPPs
  for (unsigned int i = 1; i < 3; ++i)
  {
    torch::Tensor input = torch::add(at::zeros({1, 3}, at::kDouble), i * 0.5);
    torch::Tensor prediction = nn->forward(input);

    _x_values.push_back(i * 0.5);
    _y_values.push_back(i * 0.5);
    _z_values.push_back(i * 0.5);
    _nn_values.push_back(prediction.item<double>());
  }
}

#endif
