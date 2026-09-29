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
  /**
   * Return the input parameters supported by this control.
   */
  static InputParameters validParams();

  /**
   * Construct using input parameters.
   */
  LibtorchDRLControl(const InputParameters & parameters);

  /**
   * Restore any restartable controller state after base setup completes.
   */
  virtual void initialSetup() override;

  /**
   * Compute the actions and their corresponding logarithmic probabilities.
   */
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

  /**
   * Reject loading a plain artificial neural network into this actor-based controller.
   * @param input_nn Artificial neural network passed through the base-class interface.
   */
  virtual void loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn) override;

  /**
   * Load the actor neural network from the configured checkpoint file.
   */
  virtual void loadControlNeuralNetFromFile() override;

  /**
   * Reset the owned policy-sampling generator to a known seed.
   * @param seed Seed assigned to the policy-sampling generator.
   */
  void setPolicySampleSeed(uint64_t seed);

  /**
   * Return the number of time steps between policy evaluations.
   */
  unsigned int numStepsInPeriod() const { return _num_steps_in_period; }

  /**
   * Return the number of observation time levels stacked into the actor input.
   */
  unsigned int inputTimesteps() const { return _input_timesteps; }

  /**
   * Return the number of observations read at each time level.
   */
  unsigned int numberOfObservations() const { return _observation_names.size(); }

protected:
  /**
   * Apply the current smoothed control signals, including optional offsets.
   */
  void applyControlSignals();

  /**
   * Return the offset to add to a control signal at the current time.
   * @param control_i Index of the control signal.
   */
  Real computeControlOffset(unsigned int control_i) const;

  /// The log probability of control signals from the last evaluation of the controller
  std::vector<Real> & _current_control_signal_log_probabilities;

  /// The smoothed control signal from the previous execution, saved for restart/recover.
  std::vector<Real> & _previous_control_signal;
  /// The current smoothed control signal applied to the controllable parameters.
  std::vector<Real> & _current_smoothed_signal;

  /// Constant offsets added to each control signal before it is applied.
  std::vector<Real> _control_offsets;
  /// Whether any offset data was provided by the user.
  bool _has_control_offsets;

  /// Actor network used when the controller operates as a stochastic policy.
  std::shared_ptr<Moose::LibtorchActorNeuralNet> _actor_nn;
  /// Owned libtorch CPU generator used for policy sampling.
  at::Generator _policy_generator;
  /// Restartable serialized state for the owned policy-sampling generator.
  std::vector<std::uint8_t> & _policy_generator_state;

  /// Number of time steps between policy evaluations.
  const unsigned int _num_steps_in_period;
  /// Exponential smoothing factor in (0, 1] applied to the control signal at each time step.
  const Real _control_smoothing_factor;
  /// Whether to sample actions stochastically instead of using the deterministic actor output.
  const bool _stochastic;

private:
  /**
   * Report whether the policy is evaluated at the current time step.
   */
  bool shouldEvaluatePolicy() const;

  /**
   * Restore the owned libtorch generator state from restartable storage.
   */
  void restorePolicyGeneratorState();

  /**
   * Mirror the owned libtorch generator state into restartable storage.
   */
  void savePolicyGeneratorState();
};

#endif
