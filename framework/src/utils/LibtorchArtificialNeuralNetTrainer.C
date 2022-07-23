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
#include "mpi.h"

namespace Moose
{

LibtorchArtificialNeuralNetTrainer::LibtorchArtificialNeuralNetTrainer(
    std::shared_ptr<LibtorchNeuralNet<torch::nn::Module>> nn)
  : _nn(nn)
{
}

unsigned int
LibtorchArtificialNeuralNetTrainer::computeBatchSize(unsigned int num_samples,
                                                     unsigned int num_batches)
{
  if (num_samples < num_batches)
    return 1;
  else if (num_samples % num_batches == 0)
    return num_samples / num_batches;
  else
  {
    unsigned int sample_per_batch_1 = num_samples / num_batches;
    unsigned int remainder_1 = num_samples % num_batches;
    unsigned int sample_per_batch_2 = sample_per_batch_1 - 1;
    unsigned int remainder_2 = num_batches + remainder_1;

    Real rel_remainder1 = Real(remainder_1) / Real(sample_per_batch_1);
    Real rel_remainder2 = Real(remainder_2) / Real(sample_per_batch_2);

    return rel_remainder2 > rel_remainder1 ? sample_per_batch_2 : sample_per_batch_1;
  }
}

unsigned int
LibtorchArtificialNeuralNetTrainer::computeLocalBatchSize(unsigned int batch_size,
                                                          unsigned int num_ranks,
                                                          unsigned int rank)
{
  if (batch_size < num_ranks)
    return 1;
  else
  {
    unsigned int remainder = batch_size % num_ranks;
    unsigned int num_local_samples = batch_size / num_ranks;
    if (remainder && rank < remainder)
      return num_local_samples + 1;
    else
      return num_local_samples;
  }
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
                                          const LibtorchTrainingOptions & options,
                                          const Parallel::Communicator & comm)
{
  auto t_begin = MPI_Wtime();

  int rank = comm.rank();
  int num_ranks = comm.size();

  auto num_samples = dataset.size().value();

  // We initialize a data_loader for the training part.
  unsigned int sample_per_batch = computeBatchSize(num_samples, options.num_batches);

  unsigned int sample_per_proc = computeLocalBatchSize(sample_per_batch, num_ranks, rank);

  auto transformed_data_set = dataset.map(torch::data::transforms::Stack<>());

  auto data_sampler =
      torch::data::samplers::DistributedRandomSampler(num_samples, num_ranks, rank, false);

  auto data_loader =
      torch::data::make_data_loader(std::move(transformed_data_set), data_sampler, sample_per_proc);

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

      for (auto & param : _nn->named_parameters())
      {
        MPI_Allreduce(MPI_IN_PLACE,
                      param.value().grad().data_ptr(),
                      param.value().grad().numel(),
                      MPI_DOUBLE,
                      MPI_SUM,
                      comm.get());

        param.value().grad().data() = param.value().grad().data() / num_ranks;
      }

      // Use new gradients to update the parameters
      _optimizer->step();

      epoch_loss += loss.item<double>();
    }

    MPI_Allreduce(MPI_IN_PLACE, &epoch_loss, 1, MPI_DOUBLE, MPI_SUM, comm.get());

    epoch_loss = epoch_loss / options.num_batches / num_ranks;

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

  auto t_end = MPI_Wtime();
  if (options.print_loss && rank == 0)
    Moose::out << "Neural net training time: " << COLOR_GREEN << (t_end - t_begin) << COLOR_DEFAULT
               << " s" << std::endl;
}
}

#endif
