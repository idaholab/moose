#ifdef LIBTORCH_ENABLED

#pragma once

#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchNeuralNetControl.h"

/**
 * A time-dependent, neural-network-based controller which is
 * associated with a Proximal Policy Optimization. We use this neural net for the training of a
 * controller. The additional functionality in this controller is the addition of the variability
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

  /**
   * Get the logarithmic probability of (signal_index)-th signal of the control neural net
   * @param signal_index The index of the signal
   * @return The logarithmic probability of the (signal_index)-th signal
   */
  Real getSignalLogProbability(const unsigned int signal_index) const;

protected:
  /**
   * Function which computes the logarithmic probability of given actions.
   * @param action The tensor containing the perturbed control signals (also known as the action of
   * the controller)
   * @param output_tensor The expected value of the signals predicted by the neural net
   * @return The logarithmic probability of the action with respect to the neural net prediction
   */
  torch::Tensor computeLogProbability(const torch::Tensor & action,
                                      const torch::Tensor & output_tensor);

  /// The log probability of control signals from the last evaluation of the controller
  std::vector<Real> _current_control_signal_log_probabilities;

  /// Standard deviation for the actions, supplied by the user
  const std::vector<Real> _action_std;

  /// Standard deviations converted to a 2D diagonal tensor that can be used by Libtorch routines.
  torch::Tensor _std;
};

#endif
