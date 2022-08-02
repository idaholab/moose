#pragma once

#ifdef LIBTORCH_ENABLED
#include "LibtorchNeuralNetControl.h"
#include "LibtorchArtificialNeuralNet.h"
#endif

#include "Control.h"

/**
 * A time-dependent, neural network-based control of multiple input parameters.
 * The control strategy depends on the training of the neural net, which is
 * typically done in a trainer object in the main app.
 */
class LibtorchDRLControl : public LibtorchNeuralNetControl
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  LibtorchDRLControl(const InputParameters & parameters);

  virtual void execute() override;

protected:

#ifdef LIBTORCH_ENABLED
  torch::Tensor computeLogProbability(const torch::Tensor & action,
                                      const torch::Tensor & output_tensor);
#endif

  std::vector<PostprocessorName> _log_probability_postprocessor_names;

  /// Standard deviation for the actions
  std::vector<Real> _action_std;

#ifdef LIBTORCH_ENABLED
  /// standard deviation for sampling the actual control value
  torch::Tensor _std;
#endif
};
