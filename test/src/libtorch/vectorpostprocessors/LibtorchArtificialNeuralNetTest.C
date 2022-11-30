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
#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchArtificialNeuralNetTest.h"

registerMooseObject("MooseTestApp", LibtorchArtificialNeuralNetTest);

InputParameters
LibtorchArtificialNeuralNetTest::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addParam<std::vector<std::string>>(
      "activation_functions", std::vector<std::string>({"relu"}), "Test activation functions");

  return params;
}

LibtorchArtificialNeuralNetTest::LibtorchArtificialNeuralNetTest(const InputParameters & params)
  : GeneralVectorPostprocessor(params), _nn_values(declareVector("nn_values"))
{
  torch::manual_seed(11);

  // Define neurons per hidden layer: we will have two hidden layers with 4 neurons each
  std::vector<unsigned int> num_neurons_per_layer({4, 4});
  // Create the neural network with name "test", number of input neurons = 3,
  // number of output neurons = 1, and activation functions from the input file.
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> nn =
      std::make_shared<Moose::LibtorchArtificialNeuralNet>(
          "test",
          3,
          1,
          num_neurons_per_layer,
          getParam<std::vector<std::string>>("activation_functions"));

  // Create an Adam optimizer
  torch::optim::Adam optimizer(nn->parameters(), torch::optim::AdamOptions(0.02));
  // reset the gradients
  optimizer.zero_grad();
  // This is our test input
  torch::Tensor input = at::ones({1, 3}, at::kDouble);
  // This is our test output (we know the result)
  torch::Tensor output = at::ones({1}, at::kDouble);
  // This is our prediction for the test input
  torch::Tensor prediction = nn->forward(input);
  // We save our first prediction
  _nn_values.push_back(prediction.item<double>());
  // We compute the loss
  torch::Tensor loss = torch::mse_loss(prediction, output);
  // We propagate the error back to compute gradient
  loss.backward();
  // We update the weights using the computed gradients
  optimizer.step();
  // Obtain another prediction
  prediction = nn->forward(input);
  // We save our second prediction
  _nn_values.push_back(prediction.item<double>());
}

#endif
