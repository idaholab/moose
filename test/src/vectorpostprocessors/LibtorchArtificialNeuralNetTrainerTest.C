//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED
#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchArtificialNeuralNetTrainer.h"
#include <torch/torch.h>
#endif

#include "LibtorchArtificialNeuralNetTrainerTest.h"

registerMooseObject("MooseTestApp", LibtorchArtificialNeuralNetTrainerTest);

InputParameters
LibtorchArtificialNeuralNetTrainerTest::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Tests an artificial neural net trainer.");

  params.addRequiredParam<std::string>(
      "optimizer_type", "The typo of optimizer we want to train the neural net with");
  params.addRequiredParam<unsigned int>("num_epochs", "The number of epochs we want to simulate.");
  params.addRequiredParam<unsigned int>(
      "num_batches", "The number of batches we want to split out training data into.");
  params.addRequiredParam<Real>("learning_rate", "The learning rate for the gradient descent.");
  return params;
}

LibtorchArtificialNeuralNetTrainerTest::LibtorchArtificialNeuralNetTrainerTest(
    const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _nn_values_1(declareVector("nn_values_1")),
    _nn_values_2(declareVector("nn_values_2"))
{
#ifdef LIBTORCH_ENABLED

  torch::manual_seed(11);

  unsigned int num_inputs = 3;
  unsigned int num_outputs = 2;
  unsigned int num_samples = 5;

  std::vector<unsigned int> num_neurons_per_layer({4, 4});
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> nn =
      std::make_shared<Moose::LibtorchArtificialNeuralNet>(
          "test", num_inputs, num_outputs, num_neurons_per_layer);

  std::vector<Real> data;
  std::vector<Real> results;

  for (const auto sample_index : make_range(num_samples))
  {
    for (const auto input_index : make_range(num_inputs))
      data.push_back(sample_index * input_index);

    for (const auto result_index : make_range(num_outputs))
      results.push_back(2 * sample_index * result_index + 5);
  }

  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(data.data(), {num_samples, num_inputs}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(results.data(), {num_samples, num_outputs}, options).to(at::kDouble);

  Moose::LibtorchDataset dataset(data_tensor, response_tensor);
  Moose::LibtorchArtificialNeuralNetTrainer trainer(nn);

  Moose::LibtorchTrainingOptions optim_options;
  optim_options.optimizer_type = getParam<std::string>("optimizer_type");
  optim_options.learning_rate = getParam<Real>("learning_rate");
  optim_options.num_epochs = getParam<unsigned int>("num_epochs");
  optim_options.num_batches = getParam<unsigned int>("num_batches");
  optim_options.rel_loss_tol = 1e-6;
  optim_options.print_loss = true;
  optim_options.print_epoch_loss = 20;

  trainer.train(dataset, optim_options);

  std::vector<Real> test({0.0, 1.0, 2.0});
  torch::Tensor test_tensor = torch::from_blob(test.data(), {1, 3}, options).to(at::kDouble);
  auto prediction = nn->forward(test_tensor);

  _nn_values_1.push_back(prediction[0][0].item<double>());
  _nn_values_2.push_back(prediction[0][1].item<double>());

#endif
}
