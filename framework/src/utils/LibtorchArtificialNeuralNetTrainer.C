//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED
#include "LibtorchArtificialNeuralNetTrainer.h"
#include "MooseError.h"

namespace Moose
{

LibtorchArtificialNeuralNetTrainer::LibtorchArtificialNeuralNetTrainer(
    std::shared_ptr<LibtorchNeuralNet<torch::nn::Module>> nn)
  : _nn(nn)
{
}

void
LibtorchArtificialNeuralNetTrainer::setupOptimizer(const LibtorchTrainingOptions & options)
{
  if (options.optimizer_type == "adam")
    _optimizer = std::make_unique<torch::optim::Adam>(_nn->parameters(), options.learning_rate);
  else if (options.optimizer_type == "adagrad")
    _optimizer = std::make_unique<torch::optim::Adagrad>(_nn->parameters(), options.learning_rate);
  else if (options.optimizer_type == "rmsprop")
    _optimizer = std::make_unique<torch::optim::RMSprop>(_nn->parameters(), options.learning_rate);
  else if (options.optimizer_type == "sgd")
    _optimizer = std::make_unique<torch::optim::SGD>(_nn->parameters(), options.learning_rate);
}

void
LibtorchArtificialNeuralNetTrainer::train(LibtorchDataset & dataset,
                                          const LibtorchTrainingOptions & options)
{
  auto num_samples = dataset.size().value();

  // We initialize a data_loader for the training part.
  unsigned int sample_per_batch =
      num_samples > options.num_batches ? num_samples / options.num_batches : 1;

  auto transformed_data_set = dataset.map(torch::data::transforms::Stack<>());
  auto data_loader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
      std::move(transformed_data_set), sample_per_batch);

  // Setup the optimizer
  setupOptimizer(options);

  Real rel_loss = 1.0;
  Real initial_loss = 1.0;
  Real epoch_loss = 0.0;

  // Begin training loop
  unsigned int epoch = 1;
  while (epoch <= options.num_epochs && rel_loss > options.rel_loss_tol)
  {
    epoch_loss = 0.0;
    for (auto & batch : *data_loader)
    {
      // Reset gradients
      _optimizer->zero_grad();

      // Compute prediction
      torch::Tensor prediction = _nn->forward(batch.data);

      // Compute loss values using a MSE ( mean squared error)
      torch::Tensor loss = torch::mse_loss(prediction, batch.target);

      // Propagate error back
      loss.backward();

      // Use new gradients to update the parameters
      _optimizer->step();

      epoch_loss += loss.item<double>();
    }

    epoch_loss = epoch_loss / options.num_batches;

    if (epoch == 1)
      initial_loss = epoch_loss;

    rel_loss = epoch_loss / initial_loss;

    if (options.print_loss)
      if (epoch % options.print_epoch_loss == 0 || epoch == 1)
        Moose::out << "Epoch: " << epoch << " | Loss: " << COLOR_GREEN << epoch_loss
                   << COLOR_DEFAULT << " | Rel. loss: " << COLOR_GREEN << rel_loss << COLOR_DEFAULT
                   << std::endl;
    epoch += 1;
  }
}
}

#endif
