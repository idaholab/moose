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

#include "ActiveLearningLibtorchNN.h"
#include "Standardizer.h"

#include "StochasticToolsApp.h"
#include "LoadSurrogateDataAction.h"

#include "SurrogateModelInterface.h"
#include "SurrogateTrainer.h"
#include "MooseRandom.h"

#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchArtificialNeuralNetTrainer.h"
#include "libmesh/utility.h"
#include "LibtorchUtils.h"

class ActiveLearningLibtorchNN : public SurrogateTrainerBase,
                                      public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  ActiveLearningLibtorchNN(const InputParameters & parameters);

  virtual void initialize() final{};
  virtual void execute() final{};
  virtual void reTrain(const std::vector<std::vector<Real>> & inputs,
                       const std::vector<Real> & outputs,
                       const bool & read_from_file) const final;

private:

  /// Number of neurons within the hidden layers (the length of this vector
  /// should be the same as _num_hidden_layers)
  std::vector<unsigned int> & _num_neurons_per_layer;

  /// Activation functions for each hidden layer
  std::vector<std::string> & _activation_function;

  /// Name of the pytorch output file. This is used for loading and storing
  /// already existing data.
  const std::string _nn_filename;

  /// The struct which contains the information for the training of the neural net
  Moose::LibtorchTrainingOptions _optim_options;

  /// Pointer to the neural net object (initialized as null)
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & _nn;

  /// If the training output should be standardized (scaled and shifted)
  const bool _standardize_input;

  /// If the training output should be standardized (scaled and shifted)
  const bool _standardize_output;

  /// Standardizer for use with input (x)
  StochasticTools::Standardizer & _input_standardizer;

  /// Standardizer for use with output response (y)
  StochasticTools::Standardizer & _output_standardizer;
};

#endif
