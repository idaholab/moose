//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchObservationHistory.h"
#include "Control.h"

/**
 * A time-dependent, neural network-based control of multiple input parameters.
 * The control strategy depends on the training of the neural net, which is
 * typically done in a trainer object in the main app. Alternatively, the user can read
 * neural networks using two formats:
 * 1. Torchscript format (from python)
 * 2. Regular data format containing the parameter values of the neural net
 */
class LibtorchNeuralNetControl : public Control
{
public:
  static InputParameters validParams();

  /// Construct using input parameters
  LibtorchNeuralNetControl(const InputParameters & parameters);

  /// Load any file-backed controller state after full object construction
  virtual void initialSetup() override;

  /// Execute neural network to determine the controllable parameter values
  virtual void execute() override;

  /**
   * Get the (signal_index)-th signal of the control neural net
   * @param signal_index The index of the queried control signal
   * @return The requested control signal.
   */
  Real getSignal(const unsigned int signal_index) const;

  /// Get the number of controls this object is computing
  unsigned int numberOfControlSignals() const { return _control_names.size(); }

  /**
   * Copy a trained neural network into the controller.
   * @param input_nn Neural network that should replace the currently stored controller.
   */
  virtual void loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn);

  /// Load the controller neural network from the configured file on disk.
  virtual void loadControlNeuralNetFromFile();

  /// Return a reference to the stored neural network.
  const Moose::LibtorchNeuralNetBase & controlNeuralNet() const;

  /// Return true if the object already has a neural network.
  bool hasControlNeuralNet() const { return _nn != nullptr; };

protected:
  /**
   * Check one conditional-parameter rule and raise an input error if it is violated.
   * @param param_name Main parameter that controls the rule.
   * @param conditional_param Parameters that depend on the main parameter.
   * @param should_be_defined Whether the dependent parameters should be present or absent.
   */
  void conditionalParameterError(const std::string & param_name,
                                 const std::vector<std::string> & conditional_param,
                                 bool should_be_defined = true);

  /// Refresh the current observation values from the linked postprocessors.
  void updateCurrentObservation();

  /// Build the normalized input tensor passed into the controller neural network.
  torch::Tensor prepareInputTensor();

  /// The values of the current observed postprocessor values
  std::vector<Real> _current_observation;
  /// This variable is populated if the controller needs access to older values of the
  /// observed postprocessor values
  std::vector<std::vector<Real>> & _old_observations;

  /// The names of the controllable parameters
  const std::vector<std::string> & _control_names;
  /// The control signals from the last evaluation of the controller, saved for recover/restart.
  std::vector<Real> & _current_control_signals;

  /// Names of the postprocessors which contain the observations of the system
  const std::vector<PostprocessorName> & _observation_names;

  /// Links to the current observation postprocessor values. This is necessary so that we can check
  /// if the postprocessors exist.
  std::vector<const Real *> _observation_values;

  /// Number of timesteps to use as input data from the reporters (this influences how many past
  /// results are used, e.g. the size of _old_observations)
  const unsigned int _input_timesteps;

  /// Shifting constants for the observations
  const std::vector<Real> _observation_shift_factors;
  /// Scaling constants (multipliers) for the observations
  const std::vector<Real> _observation_scaling_factors;
  /// Multipliers for the actions
  const std::vector<Real> _action_scaling_factors;

  /// Shared observation normalization and history stacking helper
  const LibtorchObservationHistory _observation_history;

  /// Pointer to the neural net object which is supposed to be used to control
  /// the parameter values. The controller owns this object, but it can be read
  /// from file or copied by a transfer.
  std::shared_ptr<Moose::LibtorchNeuralNetBase> _nn;
};

#endif
