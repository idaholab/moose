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
class LibtorchNeuralNetControl : public Control
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  LibtorchNeuralNetControl(const InputParameters & parameters);

  virtual void execute() override;

#ifdef LIBTORCH_ENABLED
  void loadControlNeuralNet(const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & input_nn);
#endif

protected:
  void conditionalParameterError(const std::string & param_name,
                                 const std::vector<std::string> & conditional_param,
                                 bool should_be_defined = true);

  std::vector<Real> _old_response;
  std::vector<Real> _current_response;

  bool _executed_once;

  std::vector<std::string> _control_names;

  std::vector<PostprocessorName> _response_names, _postprocessor_names;

  /// Shifting constants for the responses
  std::vector<Real> _response_shift_coeffs;

  /// Shifting constants for the responses
  std::vector<Real> _response_normalization_coeffs;

#ifdef LIBTORCH_ENABLED
  /// Pointer to the neural net object which is supposed to be used to control
  /// the input values
  std::shared_ptr<Moose::LibtorchNeuralNetBase> _nn;
#endif
};
