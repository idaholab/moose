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
#include "LibtorchArtificialNeuralNetTrainer.h"
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
  params.addRequiredParam<unsigned int>("num_samples",
                                        "The number of samples we want to use for training.");
  params.addRequiredParam<Real>("learning_rate", "The learning rate for the gradient descent.");
  params.addRequiredParam<std::vector<unsigned int>>("hidden_layers",
                                                     "The architecture of the hidden layers.");
  params.addRequiredParam<std::vector<Real>>("monitor_point",
                                             "The point where we want to monitor the results.");
  params.addRequiredParam<unsigned int>(
      "max_processes",
      "The maximum number of parallel processes we want to use to train the neural network.");
  return params;
}

LibtorchArtificialNeuralNetTrainerTest::LibtorchArtificialNeuralNetTrainerTest(
    const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _nn_values_1(declareVector("nn_values_1")),
    _nn_values_2(declareVector("nn_values_2"))
{
  torch::manual_seed(11);

  unsigned int num_inputs = 3;
  unsigned int num_outputs = 2;
  unsigned int num_samples = getParam<unsigned int>("num_samples");
  std::vector<unsigned int> hidden_layers = getParam<std::vector<unsigned int>>("hidden_layers");

  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> nn =
      std::make_shared<Moose::LibtorchArtificialNeuralNet>(
          "test", num_inputs, num_outputs, hidden_layers);

  std::vector<Real> data;
  std::vector<Real> results;

  for (const auto sample_index : make_range(num_samples))
  {
    std::vector<Real> sample;
    for (const auto input_index : make_range(num_inputs))
    {
      data.push_back(Real(sample_index) / Real(num_samples) + Real(input_index) / Real(num_inputs));
      sample.push_back(Real(sample_index) / Real(num_samples) +
                       Real(input_index) / Real(num_inputs));
    }

    for (const auto result_index : make_range(num_outputs))
    {
      Real radius = sqrt(sample[0] * sample[0] + sample[1] * sample[1] + sample[2] * sample[2]);
      if (radius < 1.4)
        results.push_back(0.0 + Real(result_index) / Real(num_outputs));
      else
        results.push_back(1.0 + Real(result_index) / Real(num_outputs));
    }
  }

  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(data.data(), {num_samples, num_inputs}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(results.data(), {num_samples, num_outputs}, options).to(at::kDouble);

  Moose::LibtorchDataset dataset(data_tensor, response_tensor);
  Moose::LibtorchArtificialNeuralNetTrainer<> trainer(*nn, comm());

  Moose::LibtorchTrainingOptions optim_options;
  optim_options.optimizer_type = getParam<std::string>("optimizer_type");
  optim_options.learning_rate = getParam<Real>("learning_rate");
  optim_options.num_epochs = getParam<unsigned int>("num_epochs");
  optim_options.num_batches = getParam<unsigned int>("num_batches");
  optim_options.rel_loss_tol = 1e-8;
  optim_options.print_loss = true;
  optim_options.print_epoch_loss = 20;
  optim_options.parallel_processes = getParam<unsigned int>("max_processes");

  trainer.train(dataset, optim_options);

  std::vector<Real> test(getParam<std::vector<Real>>("monitor_point"));
  torch::Tensor test_tensor = torch::from_blob(test.data(), {1, 3}, options).to(at::kDouble);
  auto prediction = nn->forward(test_tensor);

  _nn_values_1.push_back(prediction[0][0].item<double>());
  _nn_values_2.push_back(prediction[0][1].item<double>());
}

#endif
