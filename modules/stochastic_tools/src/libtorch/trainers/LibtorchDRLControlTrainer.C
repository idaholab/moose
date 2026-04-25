//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchDRLControlTrainer.h"
#include "LibtorchRandomUtils.h"

#include <cmath>
#include <numeric>

registerMooseObject("StochasticToolsApp", LibtorchDRLControlTrainer);

InputParameters
LibtorchDRLControlTrainer::validParams()
{
  InputParameters params = SurrogateTrainerBase::validParams();

  params.addClassDescription(
      "Trains a neural network controller using fixed-horizon PPO on top of the libtorch RL core.");

  params.addRequiredParam<std::vector<ReporterName>>(
      "observation", "Reporter values containing the observation values from the model.");
  params.addParam<std::vector<Real>>(
      "observation_shift_factors",
      {},
      "Optional offsets applied to the observed state values before scaling.");
  params.addParam<std::vector<Real>>(
      "observation_scaling_factors",
      {},
      "Optional multipliers applied after shifting the observed state values.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "control",
      "Reporters containing the values of the controlled quantities (control signals) from the "
      "model simulations.");
  params.addParam<std::vector<Real>>(
      "action_scaling_factors",
      {},
      "Scale factors embedded into the trained policy outputs so transferred and checkpointed "
      "controllers operate in physical units.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "log_probability",
      "Reporters containing the log probabilities of the actions taken during the simulations.");
  params.addRequiredParam<ReporterName>(
      "reward", "Reporter containing the earned time-dependent rewards from the simulation.");
  params.addRangeCheckedParam<unsigned int>(
      "input_timesteps",
      1,
      "1<=input_timesteps",
      "Number of time steps to use in the input data, if larger than 1, "
      "data from the previous timesteps will be used as inputs in the training.");
  params.addParam<unsigned int>("skip_num_rows",
                                1,
                                "Unused compatibility parameter reserved for future reporter-row "
                                "offset handling.");

  params.addRequiredParam<unsigned int>("num_epochs", "Number of epochs for the training.");

  params.addRequiredRangeCheckedParam<Real>("critic_learning_rate",
                                            "0<critic_learning_rate",
                                            "Learning rate used by the critic optimizer.");
  params.addRequiredParam<std::vector<unsigned int>>("num_critic_neurons_per_layer",
                                                     "Hidden-layer widths for the critic network.");
  params.addParam<std::vector<std::string>>(
      "critic_activation_functions",
      std::vector<std::string>({"relu"}),
      "Activation name for each critic hidden layer, or one shared value for all layers.");

  params.addRequiredRangeCheckedParam<Real>("control_learning_rate",
                                            "0<control_learning_rate",
                                            "Learning rate used by the actor optimizer.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "num_control_neurons_per_layer",
      "Number of neurons per layer for the control neural network.");
  params.addParam<std::vector<std::string>>(
      "control_activation_functions",
      std::vector<std::string>({"relu"}),
      "Activation name for each actor hidden layer, or one shared value for all layers.");

  params.addParam<std::string>("filename_base",
                               "Base filename used when writing actor and critic checkpoints.");

  params.addParam<unsigned int>(
      "seed", 11, "Random number generator seed for stochastic optimizers.");

  params.addParam<Real>(
      "clip_parameter", 0.2, "Clip parameter used while clamping the advantage value.");
  params.addRangeCheckedParam<unsigned int>(
      "update_frequency",
      1,
      "1<=update_frequency",
      "Number of transient simulation data to collect for updating the controller neural network.");

  params.addRangeCheckedParam<Real>(
      "decay_factor",
      1.0,
      "0.0<=decay_factor<=1.0",
      "Discount factor used when building PPO return and GAE targets.");

  params.addRangeCheckedParam<Real>("lambda_factor", 1.0, "0.0<=lambda_factor<=1.0", "GAE lambda.");

  params.addParam<bool>(
      "read_from_file", false, "Switch to read the neural network parameters from a file.");
  params.addParam<bool>(
      "shift_outputs",
      true,
      "Whether to shift rollout outputs so observations and actions line up in time.");
  params.addParam<bool>(
      "standardize_advantage",
      true,
      "Switch to enable the shifting and normalization of the advantages in the PPO algorithm.");
  params.addParam<unsigned int>(
      "loss_print_frequency", 0, "Print PPO loss values every N updates. Use 0 to stay quiet.");
  params.addParam<unsigned int>("batch_size", 100, "Number of flattened samples per mini-batch.");
  params.addParam<std::vector<Real>>(
      "min_control_value", {}, "Optional lower bounds for each control signal.");
  params.addParam<std::vector<Real>>(
      "max_control_value", {}, "Optional upper bounds for each control signal.");
  params.addParam<bool>(
      "state_independent_std",
      true,
      "If true, learn one Gaussian log-std parameter per action dimension. If false, learn a "
      "state-dependent std head as in the older MOOSE actor implementation.");

  params.addParam<Real>(
      "entropy_coeff", 0.01, "Entropy bonus coefficient used in the PPO actor loss.");

  params.addParam<unsigned int>(
      "timestep_window", 1, "Use every nth reporter timestep when assembling trajectories.");

  return params;
}

LibtorchDRLControlTrainer::LibtorchDRLControlTrainer(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    _state_names(getParam<std::vector<ReporterName>>("observation")),
    _state_shift_factors(isParamSetByUser("observation_shift_factors")
                             ? getParam<std::vector<Real>>("observation_shift_factors")
                             : std::vector<Real>(_state_names.size(), 0.0)),
    _state_scaling_factors(isParamSetByUser("observation_scaling_factors")
                               ? getParam<std::vector<Real>>("observation_scaling_factors")
                               : std::vector<Real>(_state_names.size(), 1.0)),
    _action_names(getParam<std::vector<ReporterName>>("control")),
    _action_scaling_factors(isParamSetByUser("action_scaling_factors")
                                ? getParam<std::vector<Real>>("action_scaling_factors")
                                : std::vector<Real>(_action_names.size(), 1.0)),
    _log_probability_names(getParam<std::vector<ReporterName>>("log_probability")),
    _reward_name(getParam<ReporterName>("reward")),
    _reward_value_pointer(&getReporterValueByName<std::vector<std::vector<Real>>>(_reward_name)),
    _input_timesteps(getParam<unsigned int>("input_timesteps")),
    _num_inputs(_input_timesteps * _state_names.size()),
    _num_outputs(_action_names.size()),
    _num_epochs(getParam<unsigned int>("num_epochs")),
    _num_critic_neurons_per_layer(
        getParam<std::vector<unsigned int>>("num_critic_neurons_per_layer")),
    _critic_learning_rate(getParam<Real>("critic_learning_rate")),
    _num_control_neurons_per_layer(
        getParam<std::vector<unsigned int>>("num_control_neurons_per_layer")),
    _control_learning_rate(getParam<Real>("control_learning_rate")),
    _update_frequency(getParam<unsigned int>("update_frequency")),
    _clip_param(getParam<Real>("clip_parameter")),
    _decay_factor(getParam<Real>("decay_factor")),
    _lambda_factor(getParam<Real>("lambda_factor")),
    _filename_base(isParamValid("filename_base") ? getParam<std::string>("filename_base") : ""),
    _read_from_file(getParam<bool>("read_from_file")),
    _shift_outputs(getParam<bool>("shift_outputs")),
    _average_episode_reward(0.0),
    _standardize_advantage(getParam<bool>("standardize_advantage")),
    _loss_print_frequency(getParam<unsigned int>("loss_print_frequency")),
    _seed(getParam<unsigned int>("seed")),
    _min_values(getParam<std::vector<Real>>("min_control_value")),
    _max_values(getParam<std::vector<Real>>("max_control_value")),
    _highest_reward(-1e8),
    _entropy_coeff(getParam<Real>("entropy_coeff")),
    _update_counter(_update_frequency),
    _timestep_window(getParam<unsigned int>("timestep_window")),
    _observation_history(_input_timesteps),
    _value_estimator(_decay_factor, _lambda_factor),
    _ppo_loss(_clip_param, _entropy_coeff)
{
  if (_state_names.size() != _state_shift_factors.size())
    paramError("observation_shift_factors",
               "The number of shift factors is not the same as the number of observations!");

  if (_state_names.size() != _state_scaling_factors.size())
    paramError("observation_scaling_factors",
               "The number of normalization coefficients is not the same as the number of "
               "observations!");

  if (_action_names.size() != _log_probability_names.size())
    paramError("log_probability",
               "The number of log-probability reporters must match the number of control "
               "reporters.");

  if (_action_names.size() != _action_scaling_factors.size())
    paramError("action_scaling_factors",
               "The number of action scaling factors must match the number of control "
               "reporters.");

  // We establish the links with the chosen reporters
  getReporterPointers(_state_names, _state_value_pointers);
  getReporterPointers(_action_names, _action_value_pointers);
  getReporterPointers(_log_probability_names, _log_probability_value_pointers);

  bool filename_valid = isParamValid("filename_base");
  const auto input_shift_factors =
      _observation_history.expandObservationFactors(_state_shift_factors);
  const auto input_scaling_factors =
      _observation_history.expandObservationFactors(_state_scaling_factors);

  // Initializing the control neural net so that the control can grab it right away
  _control_nn = std::make_shared<Moose::LibtorchActorNeuralNet>(
      filename_valid ? _filename_base + "_control.net" : "control.net",
      _num_inputs,
      _num_outputs,
      _num_control_neurons_per_layer,
      getParam<std::vector<std::string>>("control_activation_functions"),
      _min_values,
      _max_values,
      torch::kCPU,
      torch::kDouble,
      true,
      input_shift_factors,
      input_scaling_factors,
      _action_scaling_factors,
      getParam<bool>("state_independent_std"));

  // We read parameters for the control neural net if it is requested
  if (_read_from_file)
  {
    Moose::loadLibtorchActorNeuralNetState(*_control_nn, _control_nn->name());
    _console << "Loaded requested .pt file." << std::endl;
  }
  else if (filename_valid)
    torch::save(_control_nn, _control_nn->name());

  // Initialize the critic neural net
  _critic_nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      filename_valid ? _filename_base + "_ctiric.net" : "ctiric.net",
      _num_inputs,
      1,
      _num_critic_neurons_per_layer,
      getParam<std::vector<std::string>>("critic_activation_functions"),
      torch::kCPU,
      torch::kDouble,
      true,
      input_shift_factors,
      input_scaling_factors);

  _actor_optimizer = std::make_unique<torch::optim::Adam>(
      _control_nn->parameters(), torch::optim::AdamOptions(_control_learning_rate));
  _critic_optimizer = std::make_unique<torch::optim::Adam>(
      _critic_nn->parameters(), torch::optim::AdamOptions(_critic_learning_rate));

  // We read parameters for the critic neural net if it is requested
  if (_read_from_file)
  {
    try
    {
      Moose::loadLibtorchArtificialNeuralNetState(*_critic_nn, _critic_nn->name());
      _console << "Loaded requested .pt file." << std::endl;
    }
    catch (const c10::Error & e)
    {
      mooseError("The requested pytorch file could not be loaded for the critic neural net.\n",
                 e.msg());
    }
  }
  else if (filename_valid)
    torch::save(_critic_nn, _critic_nn->name());

  _control_nn->initializeNeuralNetwork(
      Moose::makeLibtorchCPUGenerator(static_cast<uint64_t>(_seed)));
  _critic_nn->initializeNeuralNetwork(
      Moose::makeLibtorchCPUGenerator(static_cast<uint64_t>(_seed) + 1));
}

void
LibtorchDRLControlTrainer::execute()
{
  collectTrajectoriesFromReporters();

  _update_counter--;

  if (_update_counter != 0 || _trajectory_buffer.empty())
    return;

  computeEpisodeRewardStatistics();

  if (_average_episode_reward > _highest_reward)
  {
    torch::save(_control_nn, _control_nn->name() + "_best");
    _highest_reward = _average_episode_reward;
  }

  _value_estimator.computeValueTargets(_trajectory_buffer, *_critic_nn);
  const auto batch = _trajectory_buffer.flatten();

  trainController(batch);
  resetData();
}

void
LibtorchDRLControlTrainer::computeEpisodeRewardStatistics()
{
  if (_trajectory_buffer.empty())
  {
    _average_episode_reward = 0.0;
    _std_episode_reward = 0.0;
    _sample_average_episode_reward.clear();
    _sample_std_episode_reward.clear();
    return;
  }

  _average_episode_reward = 0.0;
  _std_episode_reward = 0.0;
  unsigned int combined_sizes = 0;

  _sample_average_episode_reward.clear();
  _sample_std_episode_reward.clear();

  for (const auto & trajectory : _trajectory_buffer.trajectories())
  {
    const auto & sample = trajectory.rewards;
    const unsigned int sample_size = sample.size();
    if (!sample_size)
      continue;

    const Real sum = std::accumulate(sample.begin(), sample.end(), 0.0);
    const Real mean = sum / sample_size;
    _sample_average_episode_reward.push_back(mean);

    const Real variance =
        std::transform_reduce(sample.begin(),
                              sample.end(),
                              0.0,
                              std::plus<>(),
                              [mean](const Real value) { return (value - mean) * (value - mean); });
    _sample_std_episode_reward.push_back(std::sqrt(variance / sample_size));

    _average_episode_reward += sum;
    _std_episode_reward += variance;
    combined_sizes += sample_size;
  }

  if (!combined_sizes)
  {
    _average_episode_reward = 0.0;
    _std_episode_reward = 0.0;
    return;
  }

  _average_episode_reward /= combined_sizes;
  _std_episode_reward = std::sqrt(_std_episode_reward / combined_sizes);
}

void
LibtorchDRLControlTrainer::trainController(const LibtorchRLTrajectoryBuffer::TensorBatch & batch)
{
  if (!batch.size())
    return;

  // We only train on the rank 0 partition. Libtorch should still be able to
  // fetch the local threads which are available.
  if (processor_id() == 0)
  {
    auto shuffle_generator = Moose::makeLibtorchCPUGenerator(
        static_cast<uint64_t>(_seed) + static_cast<uint64_t>(_fe_problem.timeStep()));

    for (unsigned int epoch = 0; epoch < _num_epochs; ++epoch)
    {
      const auto mini_batches = _sampler.sample(
          batch, getParam<unsigned int>("batch_size"), _standardize_advantage, shuffle_generator);
      bool printed_losses = false;
      for (const auto & mini_batch : mini_batches)
      {
        const auto losses = _ppo_loss.compute(*_control_nn, *_critic_nn, mini_batch);

        _actor_optimizer->zero_grad();
        losses.actor_loss.backward();
        _actor_optimizer->step();

        _critic_optimizer->zero_grad();
        losses.critic_loss.backward();
        _critic_optimizer->step();

        if (_loss_print_frequency && epoch % _loss_print_frequency == 0 && !printed_losses)
        {
          _console << "Epoch: " << epoch << " | Actor Loss: " << COLOR_GREEN
                   << losses.actor_loss.item<double>() << COLOR_DEFAULT
                   << " | Critic Loss: " << COLOR_GREEN << losses.critic_loss.item<double>()
                   << COLOR_DEFAULT << std::endl;
          printed_losses = true;
        }
      }
    }

    _console << "Best model so far: " << _highest_reward << std::endl;
  }

  // It is time to send the trained data to every other processor so that the neural networks
  // are the same on all ranks. TODO: Make sure this can be done on a GPU as well.
  for (auto & param : _control_nn->named_parameters())
  {
    MPI_Bcast(param.value().data_ptr(), param.value().numel(), MPI_DOUBLE, 0, _communicator.get());
  }

  for (auto & param : _critic_nn->named_parameters())
  {
    MPI_Bcast(param.value().data_ptr(), param.value().numel(), MPI_DOUBLE, 0, _communicator.get());
  }

  // Save the controller neural net so our controller can read it, we also save the critic if we
  // want to continue training
  torch::save(_control_nn, _control_nn->name());
  torch::save(_critic_nn, _critic_nn->name());
}

void
LibtorchDRLControlTrainer::resetData()
{
  _trajectory_buffer.clear();
  _update_counter = _update_frequency;
}

void
LibtorchDRLControlTrainer::collectTrajectoriesFromReporters()
{
  for (const auto sample_i : index_range(*_reward_value_pointer))
  {
    const auto & reward_sample = (*_reward_value_pointer)[sample_i];
    const auto num_transitions = computeNumTransitions(reward_sample.size());
    if (!num_transitions)
      continue;

    std::vector<std::vector<Real>> observation_trajectories(_state_names.size());
    for (const auto state_i : index_range(_state_value_pointers))
      observation_trajectories[state_i] = extractDownsampledSequence(
          (*_state_value_pointers[state_i])[sample_i], 0, num_transitions + 1);

    LibtorchRLTrajectoryBuffer::Trajectory trajectory;
    trajectory.observations.reserve(num_transitions);
    trajectory.next_observations.reserve(num_transitions);
    trajectory.actions.assign(num_transitions, std::vector<Real>());
    trajectory.log_probabilities.assign(num_transitions, std::vector<Real>());

    for (auto & action_row : trajectory.actions)
      action_row.reserve(_action_names.size());
    for (auto & log_probability_row : trajectory.log_probabilities)
      log_probability_row.reserve(_log_probability_names.size());

    for (const auto step_i : make_range(num_transitions))
    {
      trajectory.observations.push_back(
          _observation_history.stackTrajectoryObservation(observation_trajectories, step_i));
      trajectory.next_observations.push_back(
          _observation_history.stackTrajectoryObservation(observation_trajectories, step_i + 1));
    }

    for (const auto action_i : index_range(_action_value_pointers))
    {
      const auto action_sequence = extractDownsampledSequence(
          (*_action_value_pointers[action_i])[sample_i], _shift_outputs, num_transitions);
      const auto log_probability_sequence = extractDownsampledSequence(
          (*_log_probability_value_pointers[action_i])[sample_i], _shift_outputs, num_transitions);

      for (const auto step_i : make_range(num_transitions))
      {
        trajectory.actions[step_i].push_back(action_sequence[step_i]);
        trajectory.log_probabilities[step_i].push_back(log_probability_sequence[step_i]);
      }
    }

    trajectory.rewards =
        extractDownsampledSequence(reward_sample, _timestep_window, num_transitions);

    _trajectory_buffer.addTrajectory(std::move(trajectory));
  }
}

unsigned int
LibtorchDRLControlTrainer::computeNumTransitions(const std::size_t raw_sequence_size) const
{
  unsigned int num_transitions = 0;
  for (std::size_t raw_index = 0; raw_index + _timestep_window < raw_sequence_size;
       raw_index += _timestep_window)
    ++num_transitions;

  return num_transitions;
}

std::vector<Real>
LibtorchDRLControlTrainer::extractDownsampledSequence(const std::vector<Real> & sample,
                                                      const unsigned int offset,
                                                      const unsigned int num_entries) const
{
  std::vector<Real> values;
  values.reserve(num_entries);

  for (const auto entry_i : make_range(num_entries))
  {
    const auto raw_index = offset + entry_i * _timestep_window;
    if (raw_index >= sample.size())
      mooseError("Reporter data is shorter than required by the configured timestep window and "
                 "history stacking.");
    values.push_back(sample[raw_index]);
  }

  return values;
}

void
LibtorchDRLControlTrainer::getReporterPointers(
    const std::vector<ReporterName> & reporter_names,
    std::vector<const std::vector<std::vector<Real>> *> & pointer_storage)
{
  pointer_storage.clear();
  for (const auto & name : reporter_names)
    pointer_storage.push_back(&getReporterValueByName<std::vector<std::vector<Real>>>(name));
}

#endif
