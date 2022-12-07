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

  /// Execute neural network to determine the controllable parameter values
  virtual void execute() override;

  /**
   * Get the (signal_index)-th signal of the control neural net
   * @param signal_index The index of the queried control signal
   * @return The (signal_index)-th constol signal
   */
  Real getSignal(const unsigned int signal_index) const;

  /// Get the number of controls this object is computing
  unsigned int numberOfControlSignals() const { return _control_names.size(); }

  /**
   * Function responsible for loading the neural network for the controller. This function is used
   * when copying the neural network from a main app which trains it.
   * @param input_nn Reference to a neural network which will be copied into this object
   */
  void loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn);

  /// Return a reference to the stored neural network
  const Moose::LibtorchNeuralNetBase & controlNeuralNet() const;

  /// Return true if the object already has a neural netwok
  bool hasControlNeuralNet() const { return (_nn != NULL); };

protected:
  /**
   * Function responsible for checking for potential user errors in the input file
   * @param param_name The name of the main parameter
   * @param conditional_param Vector parameter names that depend on the main parameter
   * @param should_be_defined If the conditional parameters should be defined when the main
   * parameter is defined
   */
  void conditionalParameterError(const std::string & param_name,
                                 const std::vector<std::string> & conditional_param,
                                 bool should_be_defined = true);

  /// Function that updates the values of the current response
  void updateCurrentResponse();

  /// Function that prepares the input tensor for the controller neural network
  torch::Tensor prepareInputTensor();

  /// The values of the current observed postprocessor values
  std::vector<Real> _current_response;
  /// This variable is populated if the controller needs acess to older values of the
  /// observed postprocessor values
  std::vector<std::vector<Real>> _old_responses;

  /// The names of the controllable parameters
  const std::vector<std::string> & _control_names;
  /// The control signals from the last evaluation of the controller
  std::vector<Real> _current_control_signals;

  /// Names of the postprocessors which contain the observations of the system
  const std::vector<PostprocessorName> & _response_names;

  /// Links to the current response postprocessor values. This is necessary so that we can check
  /// if the postprocessors exist.
  std::vector<const Real *> _response_values;

  /// Number of timesteps to use as input data from the reporters (this influences how many past
  /// results are used, e.g. the size of _old_responses)
  const unsigned int _input_timesteps;

  /// Flag to show if the vector containing the time-series of responses has been initialized or not
  bool _initialized;

  /// Shifting constants for the responses
  const std::vector<Real> _response_shift_factors;
  /// Scaling constants (multipliers) for the responses
  const std::vector<Real> _response_scaling_factors;
  /// Multipliers for the actions
  const std::vector<Real> _action_scaling_factors;

  /// Pointer to the neural net object which is supposed to be used to control
  /// the parameter values. The controller owns this object, but it can be read
  /// from file or copied by a transfer.
  std::shared_ptr<Moose::LibtorchNeuralNetBase> _nn;
};

#endif
