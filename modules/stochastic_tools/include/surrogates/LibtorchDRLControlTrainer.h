//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef LIBTORCH_ENABLED
#include <torch/torch.h>
#include "LibtorchArtificialNeuralNet.h"
#endif

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"

class LibtorchDRLControlTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  LibtorchDRLControlTrainer(const InputParameters & parameters);

  virtual void preTrain() override {}

  virtual void train() override {}

  virtual void postTrain() override;

  void trainController();

  Real averageEpisodeReward();

#ifdef LIBTORCH_ENABLED
  const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & controlNeuralNet() const
  {
    return _control_nn;
  }
#endif

protected:
  void getInputDataFromReporter(std::vector<std::vector<Real>> & data,
                                const std::vector<ReporterName> & reporter_names,
                                unsigned int num_timesteps);

  void getOutputDataFromReporter(std::vector<std::vector<Real>> & data,
                                 const std::vector<ReporterName> & reporter_names);

  void getRewardDataFromReporter(std::vector<Real> & data, const ReporterName & reporter_name);

  // function to convert input/output data from std::vector to torch::tensor
  // detatch - whether to keep the gradient info, default is to keep the gradient info
  void convertDataToTensor(std::vector<std::vector<Real>> & vector_data,
                           torch::Tensor & tensor_data,
                           const bool & detach = false);

  void convertDataToTensor(std::vector<Real> & vector_data,
                           torch::Tensor & tensor_data,
                           const bool & detach = false);

  // functions to:  evalute value - using critic model, and evaluate action (log probability of
  // action) - using actor (control) model
  torch::Tensor evaluateValue(const torch::Tensor & input);
  torch::Tensor evaluateAction(const torch::Tensor & input, const torch::Tensor & output);

  void computeDiscountedRewards();

  void resetData();

  /// Response reporter names
  std::vector<ReporterName> _response_names;

  /// Shifting constants for the responses
  std::vector<Real> _response_shift_factors;

  /// Shifting constants for the responses
  std::vector<Real> _response_scaling_factors;

  /// Control reporter names
  std::vector<ReporterName> _control_names;

  /// Log probability reporter names
  std::vector<ReporterName> _log_probability_names;

  /// Reward reporter names
  ReporterName _reward_name;

  /// Number of timesteps to use as the input data from the reporter
  unsigned int _input_timesteps;

  /// Number of rows (number of samples), input and output columns
  unsigned int _num_inputs;
  unsigned int _num_outputs;

  /// The gathered data from the reporter, each row represents one QoI, each column represents one time step
  std::vector<std::vector<Real>> _input_data;
  std::vector<std::vector<Real>> _output_data;
  std::vector<std::vector<Real>> _log_probability_data;

  /// The reward and return (reward-to-go) data. The return is calculated using the _reward_data
  std::vector<Real> _reward_data;
  std::vector<Real> _return_data;

  /// Number of epochs for the training of the emulator
  unsigned int _num_epochs;

  /// Number of batches we want to prepare for the emulator
  unsigned int _num_batches;

  /// Number of neurons within the hidden layers i nthe emulator neural net
  std::vector<unsigned int> _num_critic_neurons_per_layer;

  /// The learning rate for the optimization algorithm in the meulator
  Real _critic_learning_rate;

  /// Number of neurons within the hidden layers in the control neural net
  std::vector<unsigned int> _num_control_neurons_per_layer;

  /// The control learning rate for the optimization algorithm
  Real _control_learning_rate;

  /// Number of transient simulation data to collect before updating the controller neural net.
  const unsigned int _update_frequency;

  /// Counter for number of transient simulations to run before updating the controller
  unsigned int _update_counter;

  /// The clip parameter used while clamping the advantage value
  Real _clip_param;

  /// Decaying factor that is used when calculating the return (reward-to-go)
  const Real _decay_factor;

  /// Standard deviation for the actions
  std::vector<Real> _action_std;

  /// Name of the pytorch output file. This is used for loading and storing
  /// already existing data.
  std::string _filename_base;

  /// Switch indicating if an already existing neural net should be read from a
  /// file or not. This can be used to load existing torch files (from previous
  /// MOOSE or python runs for retraining and further manipulation)
  bool _read_from_file;

  bool _shift_outputs;

  Real _average_episode_reward;

#ifdef LIBTORCH_ENABLED
  /// Pointer to the control neural net object (initialized as null)
  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _control_nn;

  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> _critic_nn;

  /// standard deviation for sampling the actual control value
  torch::Tensor _std;

  /// Torch::tensor version of the input and action data
  torch::Tensor _input_tensor;
  torch::Tensor _output_tensor;
  torch::Tensor _return_tensor;
  torch::Tensor _log_probability_tensor;
#endif
};
