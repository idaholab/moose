#pragma once

#ifdef LIBTORCH_ENABLED
#include "LibtorchNeuralNet.h"
#include "LibtorchArtificialNeuralNet.h"
#endif

#include "Control.h"

/**
 * A time-dependent, neural network-based control of multiple input parameters.
 * The control strategy depends on the training of the neural net, which is
 * typically done in a trainer object in the main app.
 */
class LibtorchDRLControl : public Control
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  LibtorchDRLControl(const InputParameters & parameters);

  virtual void execute() override;

#ifdef LIBTORCH_ENABLED
  void loadControlNeuralNet(const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & input_nn);

  torch::Tensor computeLogProbability(const torch::Tensor & action,
                                      const torch::Tensor & output_tensor);
#endif

protected:
  void conditionalParameterError(const std::string & param_name,
                                 const std::vector<std::string> & conditional_param,
                                 bool should_be_defined = true);

  std::vector<std::vector<Real>> _old_responses;
  std::vector<Real> _current_response;

  std::vector<std::string> _control_names;

  std::vector<PostprocessorName> _response_names;
  std::vector<PostprocessorName> _action_postprocessor_names;
  std::vector<PostprocessorName> _log_probability_postprocessor_names;

  /// Number of timesteps to use as the input data from the reporter
  unsigned int _input_timesteps;

  /// Number of timesteps to use as the input data from the reporter
  bool _initialized;

  /// Shifting constants for the responses
  std::vector<Real> _response_shift_factors;

  /// Shifting constants for the responses
  std::vector<Real> _response_scaling_factors;

  /// Shifting constants for the responses
  std::vector<Real> _action_scaling_factors;

  /// Standard deviation for the actions
  std::vector<Real> _action_std;

#ifdef LIBTORCH_ENABLED
  /// Pointer to the neural net object which is supposed to be used to control
  /// the input values
  std::shared_ptr<Moose::LibtorchNeuralNetBase> _nn;
  /// standard deviation for sampling the actual control value
  torch::Tensor _std;
#endif
};
