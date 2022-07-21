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
};

// A class that describes a simple feed-forward neural net.
class LibtorchArtificialNeuralNetTrainer
{
public:
  LibtorchArtificialNeuralNetTrainer(std::shared_ptr<LibtorchNeuralNet<torch::nn::Module>> nn);

  virtual void train(LibtorchDataset & dataset, const LibtorchTrainingOptions & options);

protected:
  void setupOptimizer(const LibtorchTrainingOptions & options);

  std::shared_ptr<LibtorchNeuralNet<torch::nn::Module>> _nn;
  std::unique_ptr<torch::optim::Optimizer> _optimizer;
};

}

#endif
