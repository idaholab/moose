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

  /// We compute the actions in this function together with the corresponding logarithmic probabilities.
  virtual void execute() override;

  /**
   * Return the log-probability of one control signal from the latest actor evaluation.
   * @param signal_index Index of the queried control signal.
   * @return Log-probability of the queried signal.
   */
  Real getSignalLogProbability(const unsigned int signal_index) const;

  /**
   * Copy a trained actor into the controller.
   * @param input_nn Actor network that should replace the currently stored controller.
   */
  virtual void loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn) override;

  /// Load the actor network from the configured checkpoint file.
  virtual void loadControlNeuralNetFromFile() override;

  /// Reset the owned policy-sampling generator to a known seed.
  void setPolicySampleSeed(uint64_t seed);

protected:
  /// The log probability of control signals from the last evaluation of the controller
  std::vector<Real> _current_control_signal_log_probabilities;

  std::vector<Real> _previous_control_signal;
  std::vector<Real> _current_smoothed_signal;

  std::shared_ptr<Moose::LibtorchActorNeuralNet> _actor_nn;
  at::Generator _policy_generator;

  unsigned int _call_counter;
  const unsigned int _num_steps_in_period;
  const Real _smoother;
  const bool _stochastic;
};

#endif
