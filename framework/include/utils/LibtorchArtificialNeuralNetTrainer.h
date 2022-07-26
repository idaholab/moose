//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef LIBTORCH_ENABLED

#include "LibtorchNeuralNet.h"
#include "LibtorchDataset.h"
#include "DataIO.h"
#include "MooseEnum.h"

namespace Moose
{

struct LibtorchTrainingOptions
{
  MooseEnum optimizer_type = MooseEnum("adam adagrad rmsprop sgd", "adam");
  unsigned int num_epochs = 1;
  unsigned int num_batches = 1;
  bool print_loss = false;
  unsigned int print_epoch_loss = 1;
  Real rel_loss_tol = 1e-12;
  Real learning_rate = 1e-3;
  unsigned int parallel_processes = 1;
  bool allow_duplicates = false;
};

template <typename Sampler = torch::data::samplers::DistributedSequentialSampler>
class LibtorchArtificialNeuralNetTrainer
{
public:
  LibtorchArtificialNeuralNetTrainer(std::shared_ptr<LibtorchNeuralNet<torch::nn::Module>> nn,
                                     const Parallel::Communicator & comm);

  virtual void train(LibtorchDataset & dataset, const LibtorchTrainingOptions & options);

protected:
  void setupOptimizer(const LibtorchTrainingOptions & options);

  unsigned int computeBatchSize(unsigned int num_samples, unsigned int num_batches);
  unsigned int computeLocalBatchSize(unsigned int batch_size, unsigned int num_ranks);

  std::shared_ptr<LibtorchNeuralNet<torch::nn::Module>> _nn;
  std::unique_ptr<torch::optim::Optimizer> _optimizer;
  std::shared_ptr<Sampler> _sampler;

  const Parallel::Communicator & _comm;
};

template <typename Sampler>
LibtorchArtificialNeuralNetTrainer<Sampler>::LibtorchArtificialNeuralNetTrainer(
    std::shared_ptr<Moose::LibtorchNeuralNet<torch::nn::Module>> nn,
    const Parallel::Communicator & comm)
  : _nn(nn), _comm(comm)
{
}

template <typename Sampler>
unsigned int
LibtorchArtificialNeuralNetTrainer<Sampler>::computeBatchSize(unsigned int num_samples,
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
    unsigned int remainder_2 =
        num_samples - (num_samples / sample_per_batch_2) * sample_per_batch_2;

    Real rel_remainder1 = Real(remainder_1) / Real(sample_per_batch_1);
    Real rel_remainder2 = Real(remainder_2) / Real(sample_per_batch_2);

    return rel_remainder2 > rel_remainder1 ? sample_per_batch_2 : sample_per_batch_1;
  }
}

template <typename Sampler>
unsigned int
LibtorchArtificialNeuralNetTrainer<Sampler>::computeLocalBatchSize(unsigned int batch_size,
                                                                   unsigned int num_ranks)
{
  if (batch_size < num_ranks)
    mooseError("The number of used processors is greater than the number of samples in the batch!");
  else if (batch_size % num_ranks == 0)
    return batch_size / num_ranks;
  else
    return batch_size / num_ranks + 1;
}

template <typename Sampler>
void
LibtorchArtificialNeuralNetTrainer<Sampler>::setupOptimizer(const LibtorchTrainingOptions & options)
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

template <typename Sampler>
void
LibtorchArtificialNeuralNetTrainer<Sampler>::train(LibtorchDataset & dataset,
                                                   const LibtorchTrainingOptions & options)
{
  auto t_begin = MPI_Wtime();

  int num_ranks = std::min(_comm.size(), options.parallel_processes);
  int real_rank = _comm.rank();
  int used_rank = real_rank < num_ranks ? real_rank : 0;

  auto num_samples = dataset.size().value();

  if (num_ranks * options.num_batches > num_samples)
    mooseError("The number of used processors* number of requestedf batches " +
               std::to_string(num_ranks * options.num_batches) +
               " is greater than the number of samples used for the training!");

  // We initialize a data_loader for the training part.
  unsigned int sample_per_batch = computeBatchSize(num_samples, options.num_batches);

  unsigned int sample_per_proc = computeLocalBatchSize(sample_per_batch, num_ranks);

  auto transformed_data_set = dataset.map(torch::data::transforms::Stack<>());

  _sampler = std::make_shared<Sampler>(num_samples, num_ranks, used_rank, options.allow_duplicates);

  auto data_loader = torch::data::make_data_loader(
      std::move(transformed_data_set), *_sampler.get(), sample_per_proc);

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

      if (real_rank <= used_rank)
        epoch_loss += loss.item<double>();

      for (auto & param : _nn->named_parameters())
      {
        if (real_rank > used_rank)
          param.value().grad().data() = param.value().grad().data() * 0.0;

        MPI_Allreduce(MPI_IN_PLACE,
                      param.value().grad().data_ptr(),
                      param.value().grad().numel(),
                      MPI_DOUBLE,
                      MPI_SUM,
                      _comm.get());

        param.value().grad().data() = param.value().grad().data() / num_ranks;
      }

      // Use new gradients to update the parameters
      _optimizer->step();
    }

    MPI_Allreduce(MPI_IN_PLACE, &epoch_loss, 1, MPI_DOUBLE, MPI_SUM, _comm.get());

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
  if (options.print_loss && used_rank == 0)
    Moose::out << "Neural net training time: " << COLOR_GREEN << (t_end - t_begin) << COLOR_DEFAULT
               << " s" << std::endl;
}

template class LibtorchArtificialNeuralNetTrainer<torch::data::samplers::DistributedRandomSampler>;

template class LibtorchArtificialNeuralNetTrainer<
    torch::data::samplers::DistributedSequentialSampler>;

}

#endif
