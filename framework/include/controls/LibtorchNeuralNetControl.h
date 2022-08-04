#pragma once

#ifdef LIBTORCH_ENABLED
#include "LibtorchNeuralNet.h"
#include "LibtorchArtificialNeuralNet.h"
#endif

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

#ifdef LIBTORCH_ENABLED
  /**
   * Function responsible for loading the neural network for the controller. This function is used
   * when copying the neural network from a main app which trains it.
   */
  void loadControlNeuralNet(const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & input_nn);
#endif

protected:
  /// Function responsible for checking for potential user errors in the input file
  void conditionalParameterError(const std::string & param_name,
                                 const std::vector<std::string> & conditional_param,
                                 bool should_be_defined = true);

  /// The values of the current observed postprocessor values
  std::vector<Real> _current_response;
  /// This variable is populated if the controller needs acess to older values of the
  /// observed postprocessor values
  std::vector<std::vector<Real>> _old_responses;

  /// The names of the controllable parameters
  std::vector<std::string> _control_names;
  /// Names of the postprocessors which contain the observations of the system
  std::vector<PostprocessorName> _response_names;
  /// Names of the postprocessors which will store the resulting action values from this controller
  std::vector<PostprocessorName> _action_postprocessor_names;

  /// Number of timesteps to use as input data from the reporters (this influences how many past
  /// results are used, e.g. the size of _old_responses)
  unsigned int _input_timesteps;

  /// Number of timesteps to use as the input data from the reporter
  bool _initialized;

  /// Shifting constants for the responses
  std::vector<Real> _response_shift_factors;
  /// Scaling constants (multipliers) for the responses
  std::vector<Real> _response_scaling_factors;
  /// Multipliers for the actions
  std::vector<Real> _action_scaling_factors;

#ifdef LIBTORCH_ENABLED
  /// Pointer to the neural net object which is supposed to be used to control
  /// the parameter values
  std::shared_ptr<Moose::LibtorchNeuralNetBase> _nn;
#endif
};
