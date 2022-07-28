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
#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"
#endif

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

class LibtorchNeuralNetControlTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  LibtorchNeuralNetControlTrainer(const InputParameters & parameters);

  virtual void preTrain() override;

  virtual void train() override;

  virtual void postTrain() override;

  void trainEmulator();

  void trainController();

#ifdef LIBTORCH_ENABLED
  const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & controlNeuralNet() const
  {
    return _control_nn;
  }
#endif

private:
  /// Response reporter names
  std::vector<ReporterName> _response_names;

  /// Shifting constants for the responses
  std::vector<Real> _response_shift_coeffs;

  /// Shifting constants for the responses
  std::vector<Real> _response_normalization_coeffs;

  /// Names of the functions describing the response constraints
  std::vector<FunctionName> _response_constraints;

  /// Control reporter names
  std::vector<ReporterName> _control_names;

  /// The gathered data in a flattened form to be able to convert easily to torch::Tensor.
  std::vector<Real> _flattened_data;

  /// The gathered response in a flattened form to be able to convert easily to torch::Tensor.
  std::vector<Real> _flattened_response;

  /// Number of batches we want to prepare for the emulator
  unsigned int _num_emulator_batches;

  /// Number of epochs for the training of the emulator
  unsigned int _num_emulator_epochs;

  /// Number of neurons within the hidden layers i nthe emulator neural net
  std::vector<unsigned int> _num_emulator_neurons_per_layer;

  /// The learning rate for the optimization algorithm in the meulator
  Real _emulator_learning_rate;

  /// Number of control epochs for the training
  unsigned int _num_control_epochs;

  /// Number of control loops for the training
  unsigned int _num_control_loops;

  /// Number of neurons within the hidden layers in the control neural net
  std::vector<unsigned int> _num_control_neurons_per_layer;

  /// The control learning rate for the optimization algorithm
  Real _control_learning_rate;

  /// Name of the pytorch output file. This is used for loading and storing
  /// already existing data.
  std::string _filename;

  bool _use_old_response;

#ifdef LIBTORCH_ENABLED
  /// Pointer to the emulator neural net object (initialized as null)
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _control_nn;

  /// Pointer to the controller neural net object (initialized as null)
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _emulator_nn;
#endif
};
