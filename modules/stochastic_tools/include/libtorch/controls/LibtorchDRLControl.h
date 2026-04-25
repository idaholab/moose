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

  virtual void loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn) override;

  virtual void loadControlNeuralNetFromFile() override;

  /// Reset the owned policy-sampling generator to a known seed.
  void setPolicySampleSeed(uint64_t seed);

protected:
  /// The log probability of control signals from the last evaluation of the controller
  std::vector<Real> & _current_control_signal_log_probabilities;

  std::vector<Real> & _previous_control_signal;
  std::vector<Real> & _current_smoothed_signal;

  std::shared_ptr<Moose::LibtorchActorNeuralNet> _actor_nn;
  at::Generator _policy_generator;
  std::vector<std::uint8_t> & _policy_generator_state;

  unsigned int & _call_counter;
  const unsigned int _num_steps_in_period;
  const Real _smoother;
  const bool _stochastic;

private:
  /// Restore the owned libtorch generator state from restartable storage.
  void restorePolicyGeneratorState();

  /// Mirror the owned libtorch generator state into restartable storage.
  void savePolicyGeneratorState();
};

#endif
