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
  LibtorchArtificialNeuralNetTrainer(LibtorchArtificialNeuralNet & nn,
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
  createOptimizer(const LibtorchArtificialNeuralNet & nn, const LibtorchTrainingOptions & options);

protected:
  /// Reference to the neural network which is trained
  LibtorchArtificialNeuralNet & _nn;
};
} // end Moose namespace

#endif
