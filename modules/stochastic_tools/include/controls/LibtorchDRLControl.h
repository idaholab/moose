#pragma once

#ifdef LIBTORCH_ENABLED
#include "LibtorchArtificialNeuralNet.h"
#endif

#include "LibtorchNeuralNetControl.h"
#include "Control.h"

/**
 * A time-dependent, neural network-based which is associated with a Proximal Policy
 * Optimization. We use this neural net for the training of a controller. The
 * additional functionality in this controller is the addition of the variability
 * (using an assumed Gaussian distribution) to avoid overfitting. This control is
 * supposed to be used in conjunction with LibtorchDRLControlTrainer.
 */
class LibtorchDRLControl : public LibtorchNeuralNetControl
{
public:
  static InputParameters validParams();

  /// Construct using input parameters
  LibtorchDRLControl(const InputParameters & parameters);

  /// We compute the actions in this function together with the corresponding logarithmic probabilities.
  virtual void execute() override;

protected:
#ifdef LIBTORCH_ENABLED
  /// Function which computes the logarithmic probability of given actions.
  torch::Tensor computeLogProbability(const torch::Tensor & action,
                                      const torch::Tensor & output_tensor);
#endif

  /// The name of postprocessors which will be used to save the logarithmic probabilities to.
  std::vector<PostprocessorName> _log_probability_postprocessor_names;

  /// Standard deviation for the actions, supplied by the user
  std::vector<Real> _action_std;

#ifdef LIBTORCH_ENABLED
  /// Standard deviations converted to a 2D diagonal tensor that can be used by Libtorch routines.
  torch::Tensor _std;
#endif
};
