//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

/**
 * This trainer is responsible for training neural networks that efficiently control
 * different processes. It utilizes the Proximal Policy Optimization algorithms. For more
 * information on the algorithm, see the following resources: Schulman, John, et al. "Proximal
 * policy optimization algorithms." arXiv preprint arXiv:1707.06347 (2017).
 * https://medium.com/analytics-vidhya/coding-ppo-from-scratch-with-pytorch-part-1-4-613dfc1b14c8
 * https://stable-baselines.readthedocs.io/en/master/modules/ppo2.html
 */
class LibtorchDRLControlTrainer : public SurrogateTrainerBase
{
public:
  static InputParameters validParams();

  /// construct using input parameters
  LibtorchDRLControlTrainer(const InputParameters & parameters);

  virtual void execute() override;

  /**
   * Function which returns the current average episodic reward. It is only updated
   * at the end of every episode.
   */
  Real averageEpisodeReward() { return _average_episode_reward; }

  /// The condensed training function
  void trainController();

  const Moose::LibtorchArtificialNeuralNet & controlNeuralNet() const { return *_control_nn; }

protected:
  /// Compute the average eposiodic reward
  void computeAverageEpisodeReward();

  /**
   * Function to convert input/output data from std::vector<std::vector> to torch::tensor
   * @param vector_data The input data in vector-vectors format
   * @param tensor_data The tensor where we would like to save the results
   * @param detach If the gradient info needs to be detached from the tensor
   */
  void convertDataToTensor(std::vector<std::vector<Real>> & vector_data,
                           torch::Tensor & tensor_data,
                           const bool detach = false);

  /**
   * Function which evaluates the critic to get the value (discounter reward)
   * @param input The observation values (responses)
   * @return The estimated value
   */
  torch::Tensor evaluateValue(torch::Tensor & input);

  /**
   * Function which evaluates the control net and then computes the logarithmic probability of the
   * action
   * @param input The observation values (responses)
   * @param output The actions corresponding to the observations
   * @return The estimated value for the logarithmic probability
   */
  torch::Tensor evaluateAction(torch::Tensor & input, torch::Tensor & output);

  /// Compute the return value by discounting the rewards and summing them
  void computeRewardToGo();

  /// Reset data after updating the neural network
  void resetData();

  /// Response reporter names
  const std::vector<ReporterName> _response_names;

  /// Pointers to the current values of the responses
  std::vector<const std::vector<Real> *> _response_value_pointers;

  /// Shifting constants for the responses
  const std::vector<Real> _response_shift_factors;

  /// Scaling constants for the responses
  const std::vector<Real> _response_scaling_factors;

  /// Control reporter names
  const std::vector<ReporterName> _control_names;

  /// Pointers to the current values of the control signals
  std::vector<const std::vector<Real> *> _control_value_pointers;

  /// Log probability reporter names
  const std::vector<ReporterName> _log_probability_names;

  /// Pointers to the current values of the control log probabilities
  std::vector<const std::vector<Real> *> _log_probability_value_pointers;

  /// Reward reporter name
  const ReporterName _reward_name;

  /// Pointer to the current values of the reward
  const std::vector<Real> * _reward_value_pointer;

  /// Number of timesteps to fetch from the reporters to be the input of then eural nets
  const unsigned int _input_timesteps;

  /// Number of inputs for the control and critic neural nets
  unsigned int _num_inputs;
  /// Number of outputs for the control neural network
  unsigned int _num_outputs;

  ///@{
  /// The gathered data from the reporters, each row represents one QoI, each column represents one time step
  std::vector<std::vector<Real>> _input_data;
  std::vector<std::vector<Real>> _output_data;
  std::vector<std::vector<Real>> _log_probability_data;
  ///@}

  ///@{
  /// The reward and return data. The return is calculated using the _reward_data
  std::vector<Real> _reward_data;
  std::vector<Real> _return_data;
  ///@}

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

  /// Standard deviation for the actions
  const std::vector<Real> _action_std;

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

  /// Switch to enable the standardization of the advantages
  const bool _standardize_advantage;

  /// The frequency the loss should be printed
  const unsigned int _loss_print_frequency;

  /// Pointer to the control (or actor) neural net object
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _control_nn;
  /// Pointer to the critic neural net object
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _critic_nn;

  /// standard deviation in a tensor format for sampling the actual control value
  torch::Tensor _std;

  /// Torch::tensor version of the input and action data
  torch::Tensor _input_tensor;
  torch::Tensor _output_tensor;
  torch::Tensor _return_tensor;
  torch::Tensor _log_probability_tensor;

private:
  /**
   * Extract the response values from the postprocessors of the controlled system.
   * This assumes that they are stored in an AccumulateReporter
   * @param data The data where we would like to store the response values
   * @param reporter_names The names of the reporters which need to be extracted
   * @param num_timesteps The number of timesteps we want to use for training
   */
  void getInputDataFromReporter(std::vector<std::vector<Real>> & data,
                                const std::vector<const std::vector<Real> *> & reporter_links,
                                const unsigned int num_timesteps);
  /**
   * Extract the output (actions, logarithmic probabilities) values from the postprocessors
   * of the controlled system. This assumes that they are stored in an AccumulateReporter
   * @param data The data where we would like to store the output values
   * @param reporter_names The names of the reporters which need to be extracted
   */
  void getOutputDataFromReporter(std::vector<std::vector<Real>> & data,
                                 const std::vector<const std::vector<Real> *> & reporter_links);

  /**
   * Extract the reward values from the postprocessors of the controlled system
   * This assumes that they are stored in an AccumulateReporter.
   * @param data The data where we would like to store the reward values
   * @param reporter_names The name of the reporter which need to be extracted
   */
  void getRewardDataFromReporter(std::vector<Real> & data,
                                 const std::vector<Real> * const reporter_link);

  /// Getting reporter pointers with given names
  void getReporterPointers(const std::vector<ReporterName> & reporter_names,
                           std::vector<const std::vector<Real> *> & pointer_storage);

  /// Counter for number of transient simulations that have been run before updating the controller
  unsigned int _update_counter;
};

#endif
