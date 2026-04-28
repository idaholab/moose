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

#include <torch/torch.h>
#include "LibtorchActorNeuralNet.h"
#include "LibtorchObservationHistoryHelper.h"
#include "LibtorchRLMiniBatchSampler.h"
#include "LibtorchRLPPOLoss.h"
#include "LibtorchRLTrajectoryBuffer.h"
#include "LibtorchRLValueEstimator.h"

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

/**
 * Fixed-horizon actor-critic trainer that collects trajectories from MOOSE reporters and runs a
 * PPO update on top of reusable RL-core components (observation history, trajectory buffer,
 * mini-batch sampler, value estimator, and PPO loss).
 */
class LibtorchDRLControlTrainer : public SurrogateTrainerBase
{
public:
  static InputParameters validParams();

  /**
   * Build the PPO-based DRL trainer.
   * @param parameters Input parameters for the trainer.
   */
  LibtorchDRLControlTrainer(const InputParameters & parameters);

  /// Pull fresh rollout data from the reporters and trigger training when ready.
  virtual void execute() override;

  /**
   * Return the current average episodic reward.
   * @return Average episodic reward over the latest training window.
   */
  Real averageEpisodeReward() { return _average_episode_reward; }
  /// Return the current episodic reward standard deviation.
  Real stdEpisodeReward() { return _std_episode_reward; }

  /// Return per-sample mean episodic rewards from the latest update window.
  std::vector<Real> sampleAverageEpsiodeRewards() { return _sample_average_episode_reward; }
  /// Return per-sample episodic reward standard deviations from the latest update window.
  std::vector<Real> sampleStdEpsiodeRewards() { return _sample_std_episode_reward; }

  /**
   * Run the PPO update on a flattened on-policy batch.
   * @param batch Flattened trajectory batch to train on.
   */
  void trainController(const LibtorchRLTrajectoryBuffer::TensorBatch & batch);

  /// Return the current actor network.
  const Moose::LibtorchArtificialNeuralNet & controlNeuralNet() const { return *_control_nn; }
  /// Return the trainer seed used for sampling and shuffling.
  unsigned int seed() const { return _seed; }

protected:
  /// Compute the average episodic reward statistics for the latest samples.
  void computeEpisodeRewardStatistics();

  /// Reset the stored rollout data after an update.
  void resetData();

  /// Observation reporter names
  const std::vector<ReporterName> _state_names;

  /// Pointers to the current values of the observations
  /// We can have multiple observations, multiple samples, multiple timesteps
  std::vector<const std::vector<std::vector<Real>> *> _state_value_pointers;

  /// Shifting constants for the observations
  const std::vector<Real> _state_shift_factors;

  /// Scaling constants for the observations
  const std::vector<Real> _state_scaling_factors;

  /// Control reporter names
  const std::vector<ReporterName> _action_names;

  /// Multiplicative action scaling embedded in the actor outputs
  const std::vector<Real> _action_scaling_factors;

  /// Pointers to the current values of the control signals
  /// We can have multiple control signals, multiple samples, multiple timesteps
  std::vector<const std::vector<std::vector<Real>> *> _action_value_pointers;

  /// Log probability reporter names
  const std::vector<ReporterName> _log_probability_names;

  /// Pointers to the current values of the control log probabilities
  /// We can have multiple control signals, multiple samples, multiple timesteps
  std::vector<const std::vector<std::vector<Real>> *> _log_probability_value_pointers;

  /// Reward reporter name
  const ReporterName _reward_name;

  /// Pointer to the current values of the reward
  /// We can have multiple samples, multiple timesteps
  const std::vector<std::vector<Real>> * _reward_value_pointer;

  /// Number of timesteps to fetch from the reporters to be the input of then eural nets
  const unsigned int _input_timesteps;

  /// Number of inputs for the control and critic neural nets
  unsigned int _num_inputs;
  /// Number of outputs for the control neural network
  unsigned int _num_outputs;

  /// Number of epochs for the training of the emulator
  const unsigned int _num_epochs;

  /// Number of neurons within the hidden layers in the critic neural net
  const std::vector<unsigned int> _num_critic_neurons_per_layer;

  /// The learning rate for the optimization algorithm for the critic
  const Real _critic_learning_rate;

  /// Number of neurons within the hidden layers in the control neural net
  const std::vector<unsigned int> _num_control_neurons_per_layer;

  /// The learning rate for the optimization algorithm for the control
  const Real _control_learning_rate;

  /// Number of transients to run and collect data from before updating the controller neural net.
  const unsigned int _update_frequency;

  /// The clip parameter used while clamping the advantage value
  const Real _clip_param;

  /// Decaying factor that is used when calculating the return from the reward
  const Real _decay_factor;
  const Real _lambda_factor;

  /// Name of the pytorch output file. This is used for loading and storing
  /// already existing data
  const std::string _filename_base;

  /// Switch indicating if an already existing neural net should be read from a
  /// file or not. This can be used to load existing torch files (from previous
  /// MOOSE runs for retraining and further manipulation)
  const bool _read_from_file;

  /// Currently, the controls are executed after the user objects at initial in moose.
  /// So using a shift can realign the corresponding input-output values while reading the
  /// reporters
  const bool _shift_outputs;

  /// Storage for the current average episode reward
  Real _average_episode_reward;
  Real _std_episode_reward;

  std::vector<Real> _sample_average_episode_reward;
  std::vector<Real> _sample_std_episode_reward;

  /// Switch to enable the standardization of the advantages
  const bool _standardize_advantage;

  /// The frequency the loss should be printed
  const unsigned int _loss_print_frequency;

  /// Base seed for stochastic optimizers and policy sampling.
  const unsigned int _seed;

  /// min
  std::vector<Real> _min_values;
  /// max
  std::vector<Real> _max_values;

  /// Pointer to the control (or actor) neural net object
  std::shared_ptr<Moose::LibtorchActorNeuralNet> _control_nn;
  /// Pointer to the critic neural net object
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _critic_nn;

  Real _highest_reward;
  Real _entropy_coeff;

  std::unique_ptr<torch::optim::Adam> _actor_optimizer;
  std::unique_ptr<torch::optim::Adam> _critic_optimizer;

private:
  /**
   * Resolve reporter names into cached pointer storage.
   * @param reporter_names Reporter names to look up.
   * @param pointer_storage Output vector that receives the reporter pointers.
   */
  void getReporterPointers(const std::vector<ReporterName> & reporter_names,
                           std::vector<const std::vector<std::vector<Real>> *> & pointer_storage);

  /// Pull trajectories out of the reporters and append them to the trajectory buffer.
  void collectTrajectoriesFromReporters();

  /**
   * Figure out how many aligned transitions a raw reporter sequence contains.
   * @param raw_sequence_size Number of raw time entries in the reporter sequence.
   * @return Number of valid transitions after history stacking and downsampling.
   */
  unsigned int computeNumTransitions(std::size_t raw_sequence_size) const;

  /**
   * Downsample one raw reporter sequence into the aligned rollout sequence we train on.
   * @param sample Raw reporter sequence.
   * @param offset Starting offset used for the aligned sequence.
   * @param num_entries Number of aligned entries to extract.
   * @return Downsampled sequence.
   */
  std::vector<Real> extractDownsampledSequence(const std::vector<Real> & sample,
                                               unsigned int offset,
                                               unsigned int num_entries) const;

  /// Counter for number of transient simulations that have been run before updating the controller
  unsigned int _update_counter;

  unsigned int _timestep_window;

  /// Shared observation history stacking and factor-expansion helper
  const LibtorchObservationHistoryHelper _observation_history;
  LibtorchRLTrajectoryBuffer _trajectory_buffer;
  const LibtorchRLMiniBatchSampler _sampler;
  const LibtorchRLValueEstimator _value_estimator;
  const LibtorchRLPPOLoss _ppo_loss;
};

#endif
