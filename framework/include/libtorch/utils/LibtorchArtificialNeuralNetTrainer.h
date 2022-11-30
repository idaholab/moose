//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchDataset.h"
#include "DataIO.h"
#include "MooseEnum.h"

namespace Moose
{

/**
 * A struct containing necessary information for training neural networks.
 * This is an easy way to pass multiple parameters to the neural networks
 * and follows the design of multiple libtorch classes.
 */
struct LibtorchTrainingOptions
{
  /// The type of optimizer we want to use for training, adam is the default due to
  /// its robustness and fast convergence
  MooseEnum optimizer_type = MooseEnum("adam=0 adagrad=1 rmsprop=2 sgd=3", "adam");
  /// Number of iterations we want to perform on the whole dataset
  unsigned int num_epochs = 1;
  /// Number of batches we want to split the dataset into
  unsigned int num_batches = 1;
  /// If we want to print additional information during training
  bool print_loss = false;
  /// The frequency of training loss print to console
  unsigned int print_epoch_loss = 1;
  /// The relative loss tolerance where the training shall stop
  Real rel_loss_tol = 1e-12;
  /// The learning rate for the optimizers
  Real learning_rate = 1e-3;
  /// The number of allowed parallel processes. It is necessary to bound it because the user may
  /// want to run simulations with more processors than data samples which leads to segmentation faults
  /// is a process remains with no sample
  unsigned int parallel_processes = 1;
  /// Parameter for sampling. This actually allows random samplers to use the same sample
  /// multiple times. Sometimes comes in handy when only a few of them are needed to fill up
  /// empty slots in the last batch. NOTE: This may introduce a considerable BIAS!
  bool allow_duplicates = false;
};

/**
 * Templated class which is responsible for training LibtorchArtificialNeuralNets. The specialty
 * of this class is that it can be used with random/sequential samplers in serial or
 * parallel. The default sampling approach is sequential.
 */
template <typename SamplerType = torch::data::samplers::DistributedSequentialSampler>
class LibtorchArtificialNeuralNetTrainer : public libMesh::ParallelObject
{
public:
  /**
   * Construct using the neural network and a parallel communicator
   * @param nn The neural network which needs to be trained
   * @param comm Reference to the parallel communicator
   */
  LibtorchArtificialNeuralNetTrainer(std::shared_ptr<LibtorchArtificialNeuralNet> nn,
                                     const Parallel::Communicator & comm);

  /**
   * Train the neural network using a given (serialized) data and options for the training process
   * @param dataset The dataset containing the known input-output pairs for the training
   * @param options Options for the optimizer/training process
   */
  virtual void train(LibtorchDataset & dataset, const LibtorchTrainingOptions & options);

  /**
   * Computes the number of samples used for each batch.
   * @param num_samples The total number of samples used for the training
   * @param num_batches The total number of batches we want to select. This is just a target number,
   * based on load-balancing considerations, the code can deviate from this by 1
   * @return The number of samples in the batches
   */
  static unsigned int computeBatchSize(const unsigned int num_samples,
                                       const unsigned int num_batches);
  /**
   * Computes the number of local samples. This practically splits the already computed batches
   * between MPI ranks. At the moment, this is a very simple logic but will change in the future
   * when custom dataloaders are added.
   * @param batch_size The batch size which needs to be split
   * @param num_ranks The number of ranks associated with this batch
   * @return The number of samples in a batch which will be used locally to train the neural nets
   */
  static unsigned int computeLocalBatchSize(const unsigned int batch_size,
                                            const unsigned int num_ranks);

  /**
   * Setup the optimizer based on the provided options.
   * @param nn The neural network whose parameters need to be optimized
   * @param options The options for the training process
   */
  static std::unique_ptr<torch::optim::Optimizer>
  createOptimizer(const std::shared_ptr<LibtorchArtificialNeuralNet> nn,
                  const LibtorchTrainingOptions & options);

protected:
  /// Pointer to the neural network which is trained
  const std::shared_ptr<LibtorchArtificialNeuralNet> _nn;
};

template <typename SamplerType>
LibtorchArtificialNeuralNetTrainer<SamplerType>::LibtorchArtificialNeuralNetTrainer(
    std::shared_ptr<LibtorchArtificialNeuralNet> nn, const Parallel::Communicator & comm)
  : libMesh::ParallelObject(comm), _nn(nn)
{
}

template <typename SamplerType>
unsigned int
LibtorchArtificialNeuralNetTrainer<SamplerType>::computeBatchSize(const unsigned int num_samples,
                                                                  const unsigned int num_batches)
{
  // If we have more requested batches than the number of samples, we automatically decrease
  // the number of batches and put one sample in each
  if (num_samples < num_batches)
    return 1;
  // If the samples can be divided between the batches equally, we do that
  else if (num_samples % num_batches == 0)
    return num_samples / num_batches;
  // In all other cases, we compute the batch sizes with the specified number of batches
  // and we check if we could divide the data more evenly if we put one less sample in each
  // batch and potentially create a new batch.
  else
  {
    const unsigned int sample_per_batch_1 = num_samples / num_batches;
    const unsigned int remainder_1 = num_samples % num_batches;
    const unsigned int sample_per_batch_2 = sample_per_batch_1 - 1;
    const unsigned int remainder_2 =
        num_samples - (num_samples / sample_per_batch_2) * sample_per_batch_2;

    const Real rel_remainder1 = Real(remainder_1) / Real(sample_per_batch_1);
    const Real rel_remainder2 = Real(remainder_2) / Real(sample_per_batch_2);

    return rel_remainder2 > rel_remainder1 ? sample_per_batch_2 : sample_per_batch_1;
  }
}

template <typename SamplerType>
unsigned int
LibtorchArtificialNeuralNetTrainer<SamplerType>::computeLocalBatchSize(
    const unsigned int batch_size, const unsigned int num_ranks)
{
  // If we have more processors than the number of samples in this batch, we error out. We
  // do not support idle processors at the moment (at least not this way).
  if (batch_size < num_ranks)
    mooseError("The number of used processors is greater than the number of samples in the batch!");
  else if (batch_size % num_ranks == 0)
    return batch_size / num_ranks;
  else
    return batch_size / num_ranks + 1;
}

template <typename SamplerType>
std::unique_ptr<torch::optim::Optimizer>
LibtorchArtificialNeuralNetTrainer<SamplerType>::createOptimizer(
    const std::shared_ptr<LibtorchArtificialNeuralNet> nn, const LibtorchTrainingOptions & options)
{
  std::unique_ptr<torch::optim::Optimizer> optimizer;
  switch (options.optimizer_type)
  {
    case 0:
      optimizer = std::make_unique<torch::optim::Adam>(
          nn->parameters(), torch::optim::AdamOptions(options.learning_rate));
      break;
    case 1:
      optimizer = std::make_unique<torch::optim::Adagrad>(nn->parameters(), options.learning_rate);
      break;
    case 2:
      optimizer = std::make_unique<torch::optim::RMSprop>(nn->parameters(), options.learning_rate);
      break;
    case 3:
      optimizer = std::make_unique<torch::optim::SGD>(nn->parameters(), options.learning_rate);
      break;
  }
  return optimizer;
}

template <typename SamplerType>
void
LibtorchArtificialNeuralNetTrainer<SamplerType>::train(LibtorchDataset & dataset,
                                                       const LibtorchTrainingOptions & options)
{
  // This is used to measure the training time. Would not like to inherit from
  // PerfGraphInterface. Other objects can time this process from the outside.
  const auto t_begin = MPI_Wtime();

  /*
   * It might happen that we limit the number of processors that can be used for the praining
   * through the options argument. In this case every additional rank beyond the maximum will behave
   * as rank 0. This is necessary to avoid cases when the (number of MPI processes)*(num_batches)
   * exceeds the number of samples.
   */
  int num_ranks = std::min(n_processors(), options.parallel_processes);
  // The real rank of the current process
  int real_rank = processor_id();
  // The capped rank (or used rank) of the current process.
  int used_rank = real_rank < num_ranks ? real_rank : 0;

  const auto num_samples = dataset.size().value();

  if (num_ranks * options.num_batches > num_samples)
    mooseError("The number of used processors* number of requestedf batches " +
               std::to_string(num_ranks * options.num_batches) +
               " is greater than the number of samples used for the training!");

  // Compute the number of samples in each batch
  const unsigned int sample_per_batch = computeBatchSize(num_samples, options.num_batches);

  // Compute the number of samples for this process
  const unsigned int sample_per_proc = computeLocalBatchSize(sample_per_batch, num_ranks);

  // Transform the dataset se that the loader has an easier time
  auto transformed_data_set = dataset.map(torch::data::transforms::Stack<>());

  // Create a sampler, this is mainly here to enable random sampling. The default is sequential
  SamplerType sampler(num_samples, num_ranks, used_rank, options.allow_duplicates);

  // Generate a dataloader which will build our batches for training
  auto data_loader =
      torch::data::make_data_loader(std::move(transformed_data_set), sampler, sample_per_proc);

  // Setup the optimizer
  std::unique_ptr<torch::optim::Optimizer> optimizer = createOptimizer(_nn, options);

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
      optimizer->zero_grad();

      // Compute prediction
      torch::Tensor prediction = _nn->forward(batch.data);

      // Compute loss values using a MSE ( mean squared error)
      torch::Tensor loss = torch::mse_loss(prediction, batch.target);

      // Propagate error back
      loss.backward();

      // If we are on a process whose rank is below the allowed limit, we actually collect the loss
      if (real_rank == used_rank)
        epoch_loss += loss.item<double>();

      // To enable the parallel training, we compute the gradients of the neural net parameters
      // using backpropagation at each process and then average the gradients across processes.
      // For this we sum the data on every processor and then divide it by the active processor
      // numbers. Note: We need to zero out the gradients for inactive processors (which are beyond
      // the predefined limit)
      for (auto & param : _nn->named_parameters())
      {
        if (real_rank != used_rank)
          param.value().grad().data() = param.value().grad().data() * 0.0;

        MPI_Allreduce(MPI_IN_PLACE,
                      param.value().grad().data_ptr(),
                      param.value().grad().numel(),
                      MPI_DOUBLE,
                      MPI_SUM,
                      _communicator.get());

        param.value().grad().data() = param.value().grad().data() / num_ranks;
      }

      // Use new gradients to update the parameters
      optimizer->step();
    }

    // We also reduce the loss value to make sure every process runs the same number of epochs and
    // does not exit the loop due to hitting the realtive error condition
    _communicator.sum(epoch_loss);

    epoch_loss = epoch_loss / options.num_batches / num_ranks;

    if (epoch == 1)
      initial_loss = epoch_loss;

    rel_loss = epoch_loss / initial_loss;

    // Print training information if requested
    if (options.print_loss)
      if (epoch % options.print_epoch_loss == 0 || epoch == 1)
        Moose::out << "Epoch: " << epoch << " | Loss: " << COLOR_GREEN << epoch_loss
                   << COLOR_DEFAULT << " | Rel. loss: " << COLOR_GREEN << rel_loss << COLOR_DEFAULT
                   << std::endl;

    epoch += 1;
  }
  // This is used to measure the training time. Would not like to inherit from
  // PerfGraphInterface. Other objects can time this process from the outside.
  auto t_end = MPI_Wtime();

  if (options.print_loss && used_rank == 0)
    Moose::out << "Neural net training time: " << COLOR_GREEN << (t_end - t_begin) << COLOR_DEFAULT
               << " s" << std::endl;
}
} // end Moose namespace

#endif
