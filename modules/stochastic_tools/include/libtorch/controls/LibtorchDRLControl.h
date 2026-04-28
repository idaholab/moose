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

#include <cstdint>

#include "LibtorchActorNeuralNet.h"
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

  /// Restore any restartable controller state after base setup completes.
  virtual void initialSetup() override;

  /// We compute the actions in this function together with the corresponding logarithmic probabilities.
  virtual void execute() override;

  /**
   * Get the logarithmic probability of (signal_index)-th signal of the control neural net
   * @param signal_index The index of the signal
   * @return The logarithmic probability of the (signal_index)-th signal
   */
  Real getSignalLogProbability(const unsigned int signal_index) const;

  /**
   * Copy an actor network into this DRL controller.
   * @param input_nn Actor network to copy into the controller.
   */
  void loadControlNeuralNet(const Moose::LibtorchActorNeuralNet & input_nn);

  virtual void loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn) override;

  virtual void loadControlNeuralNetFromFile() override;

  /// Reset the owned policy-sampling generator to a known seed.
  void setPolicySampleSeed(uint64_t seed);

protected:
  /// The log probability of control signals from the last evaluation of the controller
  std::vector<Real> & _current_control_signal_log_probabilities;

  /// The smoothed control signal from the previous execution, saved for restart/recover.
  std::vector<Real> & _previous_control_signal;
  /// The current smoothed control signal applied to the controllable parameters.
  std::vector<Real> & _current_smoothed_signal;

  /// Actor network used when the controller operates as a stochastic policy.
  std::shared_ptr<Moose::LibtorchActorNeuralNet> _actor_nn;
  /// Owned libtorch CPU generator used for policy sampling.
  at::Generator _policy_generator;
  /// Restartable serialized state for the owned policy-sampling generator.
  std::vector<std::uint8_t> & _policy_generator_state;

  /// Number of controller executions since initialization or restart.
  unsigned int & _call_counter;
  /// Number of executions to reuse a sampled action before evaluating the policy again.
  const unsigned int _num_steps_in_period;
  /// Relaxation factor applied while smoothing control updates.
  const Real _smoother;
  /// Whether to sample actions stochastically instead of using the deterministic actor output.
  const bool _stochastic;

private:
  /// Restore the owned libtorch generator state from restartable storage.
  void restorePolicyGeneratorState();

  /// Mirror the owned libtorch generator state into restartable storage.
  void savePolicyGeneratorState();
};

#endif
