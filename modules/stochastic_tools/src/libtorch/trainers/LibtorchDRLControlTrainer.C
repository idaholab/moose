//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchDataset.h"
#include "LibtorchUtils.h"
#include "LibtorchDRLControlTrainer.h"
#include "Sampler.h"
#include "Function.h"

registerMooseObject("StochasticToolsApp", LibtorchDRLControlTrainer);

InputParameters
LibtorchDRLControlTrainer::validParams()
{
  InputParameters params = SurrogateTrainerBase::validParams();

  params.addClassDescription(
      "Trains a neural network controller using the Proximal Policy Optimization (PPO) algorithm.");

  params.addRequiredParam<std::vector<ReporterName>>(
      "response", "Reporter values containing the response values from the model.");
  params.addParam<std::vector<Real>>(
      "response_shift_factors",
      "A shift constant which will be used to shift the response values. This is used for the "
      "manipulation of the neural net inputs for better training efficiency.");
  params.addParam<std::vector<Real>>(
      "response_scaling_factors",
      "A normalization constant which will be used to divide the response values. This is used for "
      "the manipulation of the neural net inputs for better training efficiency.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "control",
      "Reporters containing the values of the controlled quantities (control signals) from the "
      "model simulations.");
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
                                "Number of rows to ignore from training. We usually skip the 1st "
                                "row from the reporter since it contains only initial values.");

  params.addRequiredParam<unsigned int>("num_epochs", "Number of epochs for the training.");

  params.addRequiredRangeCheckedParam<Real>(
      "critic_learning_rate",
      "0<critic_learning_rate",
      "Learning rate (relaxation) for the emulator training.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "num_critic_neurons_per_layer", "Number of neurons per layer in the emulator neural net.");
  params.addParam<std::vector<std::string>>(
      "critic_activation_functions",
      std::vector<std::string>({"relu"}),
      "The type of activation functions to use in the emulator neural net. It is either one value "
      "or one value per hidden layer.");

  params.addRequiredRangeCheckedParam<Real>(
      "control_learning_rate",
      "0<control_learning_rate",
      "Learning rate (relaxation) for the control neural net training.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "num_control_neurons_per_layer",
      "Number of neurons per layer for the control neural network.");
  params.addParam<std::vector<std::string>>(
      "control_activation_functions",
      std::vector<std::string>({"relu"}),
      "The type of activation functions to use in the control neural net. It "
      "is either one value "
      "or one value per hidden layer.");

  params.addParam<std::string>("filename_base",
                               "Filename used to output the neural net parameters.");

  params.addParam<unsigned int>(
      "seed", 11, "Random number generator seed for stochastic optimizers.");

  params.addRequiredParam<std::vector<Real>>(
      "action_standard_deviations", "Standard deviation value used while sampling the actions.");

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
      "Decay factor for calculating the return. This accounts for decreased "
      "reward values from the later steps.");

  params.addRangeCheckedParam<Real>(
        "lambda_factor",
        1.0,
        "0.0<=lambda_factor<=1.0",
        "GAE lambda.");

  params.addParam<bool>(
      "read_from_file", false, "Switch to read the neural network parameters from a file.");
  params.addParam<bool>(
      "shift_outputs",
      true,
      "If we would like to shift the outputs the realign the input-output pairs.");
  params.addParam<bool>(
      "standardize_advantage",
      true,
      "Switch to enable the shifting and normalization of the advantages in the PPO algorithm.");
  params.addParam<unsigned int>("loss_print_frequency",
                                0,
                                "The frequency which is used to print the loss values. If 0, the "
                                "loss values are not printed.");
  params.addParam<unsigned int>("batch_size", 100, "Batch size");
  params.addParam<std::vector<Real>>("min_control_value", {}, "The minimum values of the control signal.");
  params.addParam<std::vector<Real>>("max_control_value", {}, "The maximum calue of the control signal.");

  params.addParam<unsigned int>("timestep_window", 1, "Data acquisition timesteps (every nth)");

  return params;
}

LibtorchDRLControlTrainer::LibtorchDRLControlTrainer(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    _state_names(getParam<std::vector<ReporterName>>("response")),
    _state_shift_factors(isParamValid("response_shift_factors")
                                ? getParam<std::vector<Real>>("response_shift_factors")
                                : std::vector<Real>(_state_names.size(), 0.0)),
    _state_scaling_factors(isParamValid("response_scaling_factors")
                                  ? getParam<std::vector<Real>>("response_scaling_factors")
                                  : std::vector<Real>(_state_names.size(), 1.0)),
    _action_names(getParam<std::vector<ReporterName>>("control")),
    _log_probability_names(getParam<std::vector<ReporterName>>("log_probability")),
    _reward_name(getParam<ReporterName>("reward")),
    _reward_value_pointer(&getReporterValueByName<std::vector<std::vector<Real>>>(_reward_name)),
    _input_timesteps(getParam<unsigned int>("input_timesteps")),
    _num_inputs(_input_timesteps * _state_names.size()),
    _num_outputs(_action_names.size()),
    _state_data(std::vector<std::vector<std::vector<Real>>>(_num_inputs)),
    _next_state_data(std::vector<std::vector<std::vector<Real>>>(_num_inputs)),
    _action_data(std::vector<std::vector<std::vector<Real>>>(_num_outputs)),
    _log_probability_data(std::vector<std::vector<std::vector<Real>>>(_num_outputs)),
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
    _action_std(getParam<std::vector<Real>>("action_standard_deviations")),
    _filename_base(isParamValid("filename_base") ? getParam<std::string>("filename_base") : ""),
    _read_from_file(getParam<bool>("read_from_file")),
    _shift_outputs(getParam<bool>("shift_outputs")),
    _average_episode_reward(0.0),
    _standardize_advantage(getParam<bool>("standardize_advantage")),
    _loss_print_frequency(getParam<unsigned int>("loss_print_frequency")),
    _min_values(getParam<std::vector<Real>>("min_control_value")),
    _max_values(getParam<std::vector<Real>>("max_control_value")),
    _update_counter(_update_frequency),
    _timestep_window(getParam<unsigned int>("timestep_window"))
{
  if (_state_names.size() != _state_shift_factors.size())
    paramError("response_shift_factors",
               "The number of shift factors is not the same as the number of responses!");

  if (_state_names.size() != _state_scaling_factors.size())
    paramError(
        "response_scaling_factors",
        "The number of normalization coefficients is not the same as the number of responses!");

  // We establish the links with the chosen reporters
  getReporterPointers(_state_names, _state_value_pointers);
  getReporterPointers(_action_names, _action_value_pointers);
  getReporterPointers(_log_probability_names, _log_probability_value_pointers);

  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(getParam<unsigned int>("seed"));

  bool filename_valid = isParamValid("filename_base");

  // Initializing the control neural net so that the control can grab it right away
  _control_nn = std::make_shared<Moose::LibtorchActorNeuralNet>(
      filename_valid ? _filename_base + "_control.net" : "control.net",
      _num_inputs,
      _num_outputs,
      _num_control_neurons_per_layer,
      _action_std,
      getParam<std::vector<std::string>>("control_activation_functions"),
      _min_values,
      _max_values);

  // We read parameters for the control neural net if it is requested
  if (_read_from_file)
  {
    try
    {
      torch::load(_control_nn, _control_nn->name());
      _console << "Loaded requested .pt file." << std::endl;
    }
    catch (const c10::Error & e)
    {
      mooseError("The requested pytorch file could not be loaded for the control neural net.\n",
                 e.msg());
    }
  }
  else if (filename_valid)
    torch::save(_control_nn, _control_nn->name());

  // Initialize the critic neural net
  _critic_nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      filename_valid ? _filename_base + "_ctiric.net" : "ctiric.net",
      _num_inputs,
      1,
      _num_critic_neurons_per_layer,
      getParam<std::vector<std::string>>("critic_activation_functions"));

  _actor_optimizer = std::make_unique<torch::optim::Adam>(_control_nn->parameters(),
    torch::optim::AdamOptions(_control_learning_rate));
  _critic_optimizer = std::make_unique<torch::optim::Adam>(_critic_nn->parameters(),
    torch::optim::AdamOptions(_critic_learning_rate));

  // We read parameters for the critic neural net if it is requested
  if (_read_from_file)
  {
    try
    {
      torch::load(_critic_nn, _critic_nn->name());
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

  // Define the optimizers for the training
  // torch::optim::Adam actor_optimizer(_control_nn->parameters(),
  // torch::optim::AdamOptions(_control_learning_rate));

  // torch::optim::Adam critic_optimizer(_critic_nn->parameters(),
  // torch::optim::AdamOptions(_critic_learning_rate));

  // auto obs = torch::zeros({4,10}, torch::TensorOptions().dtype(torch::kFloat64));
  // for (int i : make_range(10))
  //   for (int j : make_range(4))
  //     obs.index_put_({j, i}, j+0.1*(i+1));

  // auto action = torch::zeros({4,1}, torch::TensorOptions().dtype(torch::kFloat64));
  // for (int j : make_range(4))
  //   action.index_put_({j, 0}, 0.01+j*0.005);

  // auto log_prob = torch::zeros({4,1}, torch::TensorOptions().dtype(torch::kFloat64));
  // for (int j : make_range(4))
  //   log_prob.index_put_({j, 0}, 2.3-j*0.2);

  // auto reward = torch::zeros({4,1}, torch::TensorOptions().dtype(torch::kFloat64));
  // for (int j : make_range(4))
  //   reward.index_put_({j, 0}, -2.9-j*0.1);

  // auto ret = torch::zeros({4,1}, torch::TensorOptions().dtype(torch::kFloat64));
  // Real v = 0.0;
  // for (int j : make_range(4))
  // {
  //   v = reward.index({3-j, 0}).item<Real>()+0.95*v;
  //   ret.index_put_({3-j, 0}, v);
  // }

  // std::cout << "states" << std::endl;
  // std::cout << obs << std::endl;
  // std::cout << "actions" << std::endl;
  // std::cout << action << std::endl;
  // std::cout << "logprobs" << std::endl;
  // std::cout << log_prob << std::endl;
  // std::cout << "reward" << std::endl;
  // std::cout << reward << std::endl;
  // std::cout << "return" << std::endl;
  // std::cout << ret << std::endl;

  // auto value = evaluateValue(obs).detach();

  // std::cout << "evaluate V" << std::endl;
  // std::cout << value << std::endl;

  // auto advantage = ret - value;

  // std::cout << "advantage" << std::endl;
  // std::cout << advantage << std::endl;

  // // Get the approximate return from the neural net again (this one does have an associated
  // // gradient)
  // value = evaluateValue(obs);

  // auto new_action = _control_nn->evaluate(obs, true);

  // std::cout << "new action" << std::endl;
  // std::cout << new_action << std::endl;

  // // std::cout << "new action " << new_action << std::endl;
  // // Get the approximate logarithmic action probability using the control neural net
  // auto curr_log_probability = _control_nn->logProbability(action);

  // // std::cout << "log probability " << curr_log_probability << std::endl;

  // // Prepare the ratio by using the e^(logx-logy)=x/y expression
  // auto ratio = (curr_log_probability - log_prob).exp();

  // std::cout << "ratio" << std::endl;
  // std::cout << ratio << std::endl;

  // // Use clamping for limiting
  // auto surr1 = ratio * advantage;
  // auto surr2 = torch::clamp(ratio, 1.0 - _clip_param, 1.0 + _clip_param) * advantage;

  // // Compute loss values for the critic and the control neural net
  // auto actor_loss = -(torch::min(surr1, surr2) + 0.01*_control_nn->entropy()).mean();
  // auto critic_loss = torch::mse_loss(value, ret);

  // std::cout << "actor loss" << std::endl;
  // std::cout << actor_loss << std::endl;

  // std::cout << "critic loss" << std::endl;
  // std::cout << critic_loss << std::endl;

  // // Update the weights in the neural nets
  // actor_optimizer.zero_grad();
  // actor_loss.backward();
  // actor_optimizer.step();

  // critic_optimizer.zero_grad();
  // critic_loss.backward();
  // critic_optimizer.step();

  _control_nn->initializeNeuralNetwork();

  // std::cout << "Control NN" << std::endl;
  // const auto & control_params = _control_nn->named_parameters();
  // for (const auto & param_i : make_range(control_params.size()))
  // {
  //   // We cast the parameters into a 1D vector
  //   std::cout << Moose::stringify(std::vector<Real>(
  //       control_params[param_i].value().data_ptr<Real>(),
  //       control_params[param_i].value().data_ptr<Real>() +control_params[param_i].value().numel())) << std::endl;
  // }

  _critic_nn->initializeNeuralNetwork();

  // std::cout << "Critic NN" << std::endl;
  // const auto & critic_params = _critic_nn->named_parameters();
  // for (const auto & param_i : make_range(critic_params.size()))
  // {
  //   // We cast the parameters into a 1D vector
  //   std::cout << Moose::stringify(std::vector<Real>(
  //     critic_params[param_i].value().data_ptr<Real>(),
  //     critic_params[param_i].value().data_ptr<Real>() + critic_params[param_i].value().numel())) << std::endl;
  // }

  // mooseError("Bazinga");
}

void
LibtorchDRLControlTrainer::execute()
{
  // Extract data from the reporters
  getResponseDataFromReporter(_state_data, _next_state_data, _state_value_pointers, _input_timesteps);
  getSignalDataFromReporter(_action_data, _action_value_pointers);
  getSignalDataFromReporter(_log_probability_data, _log_probability_value_pointers);
  getRewardDataFromReporter(_reward_data, _reward_value_pointer);

  _update_counter--;

  // Only update the NNs when
  if (_update_counter == 0)
  {
    // Calculate return from the reward (discounting the reward)
    computeReturn(_return_data, _reward_data, _decay_factor);

    // We compute the average reward first
    computeAverageEpisodeReward();

    normalizeResponseData(_state_data, _input_timesteps);
    normalizeResponseData(_next_state_data, _input_timesteps);

    computeCumulativeRewardEstimate(_delta_data, _state_data, _next_state_data, _reward_data);

    computeReturn(_gae_data, _delta_data, _decay_factor*_lambda_factor);

    // Transform input/output/return data to torch::Tensor
    convertDataToTensor(_state_data, _state_tensor);
    convertDataToTensor(_next_state_data, _next_state_tensor);
    convertDataToTensor(_action_data, _action_tensor);
    convertDataToTensor(_log_probability_data, _log_probability_tensor);

    // Discard (detach) the gradient info for return data
    convertDataToTensor(_return_data, _return_tensor, true);
    convertDataToTensor(_gae_data, _gae_tensor, true);

    // We train the controller using the emulator to get a good control strategy
    trainController();

    // We clean the training data after controller update and reset the counter
    resetData();
  }
}

void
LibtorchDRLControlTrainer::computeAverageEpisodeReward()
{
  if (_reward_data.size())
  {
    _average_episode_reward = 0.0;
    unsigned int combined_sizes = 0;
    for (const auto & sample : _reward_data)
    {
      _average_episode_reward +=
        std::accumulate(sample.begin(), sample.end(), 0.0);
      combined_sizes += sample.size();
    }
    _average_episode_reward = _average_episode_reward/combined_sizes;
  }
  else
    _average_episode_reward = 0.0;
}

void
LibtorchDRLControlTrainer::computeReturn(std::vector<std::vector<Real>> & data,
                                         const std::vector<std::vector<Real>> & reward,
                                         const Real decay_factor)
{
  // Discount the reward to get the return value, we need this to be able to anticipate
  // rewards based on the current behavior. We go backwards in samples and backwards in
  // accumulation.
  for (const auto sample_i : index_range(reward))
  {
    std::vector<Real> sample_return;
    Real discounted_reward(0.0);
    const auto sample_size = reward[sample_i].size();
    for (const auto time_i : make_range(sample_size))
    {
      discounted_reward = reward[sample_i][sample_size - time_i - 1] + discounted_reward * decay_factor;

      // We are inserting to the front of the vector and push the rest back, this will
      // ensure that the first element of the vector is the discounter reward for the whole transient
      sample_return.insert(sample_return.begin(), discounted_reward);
    }

    // Save and accumulate the return values
    data.push_back(std::move(sample_return));
  }
}

void
LibtorchDRLControlTrainer::computeCumulativeRewardEstimate(std::vector<std::vector<Real>> & data,
  std::vector<std::vector<std::vector<Real>>> & state,
  std::vector<std::vector<std::vector<Real>>> & next_state,
  std::vector<std::vector<Real>> & reward)
{
  for (const auto sample_i : index_range(reward))
  {
    torch::Tensor observations;
    torch::Tensor next_observations;
    torch::Tensor reward_tensor;

    LibtorchUtils::vectorToTensor(reward[sample_i], reward_tensor, true);

    for (const auto feature_i : index_range(state))
    {
      torch::Tensor input_row;
      torch::Tensor next_input_row;
      LibtorchUtils::vectorToTensor(state[feature_i][sample_i], input_row, true);
      LibtorchUtils::vectorToTensor(next_state[feature_i][sample_i], next_input_row, true);

      if (feature_i == 0)
      {
        observations = input_row;
        next_observations = next_input_row;
      }
      else
      {
        observations = torch::cat({observations, input_row}, 1);
        next_observations = torch::cat({next_observations, next_input_row}, 1);
      }
    }

    // std::cout << "going to GAE" << std::endl;
    // std::cout << observations << std::endl;
    // std::cout << next_observations << std::endl;


    auto value = evaluateValue(observations).detach();
    auto value_next = evaluateValue(next_observations).detach();

    // std::cout << "values" << std::endl;
    // std::cout << value << std::endl;
    // std::cout << value_next << std::endl;

    auto delta = reward_tensor + _decay_factor*value_next - value;

    // std::cout << "delta" << std::endl;
    // std::cout << delta << std::endl;

    std::vector<Real> delta_vector;
    LibtorchUtils::tensorToVector(delta, delta_vector);

    data.push_back(std::move(delta_vector));
  }
}

void
LibtorchDRLControlTrainer::trainController()
{
  // We only train on the rank 0 partition. Libtorch should still be able to
  // fetch the local threads which are available.
  if (processor_id() == 0)
  {
    // std::cout << "Training" << std::endl;
    // std::cout << "Input tensor" << std::endl << _state_tensor << std::endl;
    // std::cout << "Input tensor" << std::endl << _next_state_tensor << std::endl;
    // std::cout << "Signal tensor" << std::endl << _action_tensor << std::endl;
    // std::cout << "Logprob tensor" << std::endl << _log_probability_tensor << std::endl;
    // std::cout << "reward" << std::endl << Moose::stringify(_reward_data) << std::endl;
    // std::cout << "Return tensor" << std::endl << _return_tensor << std::endl;
    // std::cout << "GAE" << std::endl << _gae_tensor << std::endl;

    // Define the optimizers for the training
    // torch::optim::Adam actor_optimizer(_control_nn->parameters(),
    //                                   torch::optim::AdamOptions(_control_learning_rate));

    // torch::optim::Adam critic_optimizer(_critic_nn->parameters(),
    //                                     torch::optim::AdamOptions(_critic_learning_rate));

    // Compute the approximate value (return) from the critic neural net and use it to compute an
    // advantage

    // Transform the dataset se that the loader has an easier time
    auto input_size = _state_tensor.sizes()[0];
    auto batch_size = getParam<unsigned int>("batch_size");
    // auto data_loader = torch::data::make_data_loader(std::move(transformed_data_set), batch_size);

    for (unsigned int epoch = 0; epoch < _num_epochs; ++epoch)
    {
      auto permutation = torch::randperm(input_size);
      unsigned int batch_begin = 0;
      unsigned int batch_end = 0;
      while (batch_end < input_size)
      {
        batch_end = batch_begin + batch_size > input_size ? input_size : batch_begin + batch_size;
        unsigned int offset = batch_end - batch_begin;
        auto batch_permutation = permutation.narrow(0, batch_begin, offset);
        auto obs_batch = _state_tensor.index({batch_permutation});
        auto action_batch = _action_tensor.index({batch_permutation});
        auto log_prob_batch = _log_probability_tensor.index({batch_permutation});
        auto return_batch = _return_tensor.index({batch_permutation});
        auto advantage_batch = _gae_tensor.index({batch_permutation});

        if (_standardize_advantage)
          advantage_batch = (advantage_batch - advantage_batch.mean()) / (advantage_batch.std() + 1e-10);

        // Get the approximate return from the neural net again (this one does have an associated
        // gradient)
        auto value = evaluateValue(obs_batch);

        auto new_action = _control_nn->evaluate(obs_batch, false);

        // std::cout << "new action " << new_action << std::endl;
        // Get the approximate logarithmic action probability using the control neural net
        auto curr_log_probability = _control_nn->logProbability(action_batch);

        // std::cout << "log probability " << curr_log_probability << std::endl;

        // Prepare the ratio by using the e^(logx-logy)=x/y expression
        auto ratio = (curr_log_probability - log_prob_batch).exp();

        // Use clamping for limiting
        auto surr1 = ratio * advantage_batch;
        auto surr2 = torch::clamp(ratio, 1.0 - _clip_param, 1.0 + _clip_param) * advantage_batch;

        // Compute loss values for the critic and the control neural net
        auto actor_loss = -(torch::min(surr1, surr2) + 0.01*_control_nn->entropy()).mean();
        auto critic_loss = torch::mse_loss(value, return_batch);

        // Update the weights in the neural nets
        _actor_optimizer->zero_grad();
        actor_loss.backward();
        _actor_optimizer->step();

        _critic_optimizer->zero_grad();
        critic_loss.backward();
        _critic_optimizer->step();

        // std::cout << "Control NN" << std::endl;
        // const auto & control_params = _control_nn->named_parameters();
        // for (const auto & param_i : make_range(control_params.size()))
        // {
        //   // We cast the parameters into a 1D vector
        //   std::cout << Moose::stringify(std::vector<Real>(
        //       control_params[param_i].value().data_ptr<Real>(),
        //       control_params[param_i].value().data_ptr<Real>() +control_params[param_i].value().numel())) << std::endl;
        // }

        // std::cout << "Critic NN" << std::endl;
        // const auto & critic_params = _critic_nn->named_parameters();
        // for (const auto & param_i : make_range(critic_params.size()))
        // {
        //   // We cast the parameters into a 1D vector
        //   std::cout << Moose::stringify(std::vector<Real>(
        //     critic_params[param_i].value().data_ptr<Real>(),
        //     critic_params[param_i].value().data_ptr<Real>() + critic_params[param_i].value().numel())) << std::endl;
        // }

              // print loss per epoch
      if (_loss_print_frequency)
        if (epoch % _loss_print_frequency == 0 && batch_begin == 0)
        {
          _console << "Epoch: " << epoch << " | Actor Loss: " << COLOR_GREEN
                << actor_loss.item<double>() << COLOR_DEFAULT << " | Critic Loss: " << COLOR_GREEN
                << critic_loss.item<double>() << COLOR_DEFAULT << std::endl;
        }

        batch_begin = batch_end;
      }
      // std::cout << _control_nn->stdTensor() << std::endl;
      std::cout << _control_nn->alphaTensor().mean() << std::endl;
      std::cout << _control_nn->betaTensor().mean() << std::endl;
    }
  }

  // It is time to send the trained data to every other processor so that the neural networks
  // are the same on all ranks. TODO: Make sure this can be done on a GPU as well.
  for (auto & param : _control_nn->named_parameters())
  {
    MPI_Bcast(param.value().data_ptr(),
    param.value().numel(),
    MPI_DOUBLE,
    0,
    _communicator.get());
  }

  for (auto & param : _critic_nn->named_parameters())
  {
    MPI_Bcast(param.value().data_ptr(),
    param.value().numel(),
    MPI_DOUBLE,
    0,
    _communicator.get());
  }

  // Save the controller neural net so our controller can read it, we also save the critic if we
  // want to continue training
  torch::save(_control_nn, _control_nn->name());
  torch::save(_critic_nn, _critic_nn->name());
}

void
LibtorchDRLControlTrainer::convertDataToTensor(std::vector<std::vector<std::vector<Real>>> & vector_data,
                                               torch::Tensor & tensor_data,
                                               const bool detach)
{
  for (const auto feature_i : index_range(vector_data))
  {
    if (vector_data[feature_i].size())
    {
      torch::Tensor concatenated_feature;
      convertDataToTensor(vector_data[feature_i], concatenated_feature, detach);

      if (feature_i == 0)
        tensor_data = concatenated_feature;
      else
        tensor_data = torch::cat({tensor_data, concatenated_feature}, 1);
    }
  }

  if (detach)
    tensor_data.detach();
}

void
LibtorchDRLControlTrainer::convertDataToTensor(std::vector<std::vector<Real>> & vector_data,
                                               torch::Tensor & tensor_data,
                                               const bool detach)
{
  if (vector_data.size())
  {
    for (const auto vector_i : index_range(vector_data))
    {
      torch::Tensor input_row;
      LibtorchUtils::vectorToTensor(vector_data[vector_i], input_row, detach);

      if (vector_i == 0)
        tensor_data = input_row;
      else
        tensor_data = torch::cat({tensor_data, input_row}, 0);
    }

    if (detach)
      tensor_data.detach();
  }
}

torch::Tensor
LibtorchDRLControlTrainer::evaluateValue(torch::Tensor & input)
{
  return _critic_nn->forward(input);
}

void
LibtorchDRLControlTrainer::resetData()
{
  for (auto & data : _state_data)
    data.clear();
  for (auto & data : _next_state_data)
    data.clear();
  for (auto & data : _action_data)
    data.clear();
  for (auto & data : _log_probability_data)
    data.clear();

  _reward_data.clear();
  _return_data.clear();
  _gae_data.clear();
  _delta_data.clear();


  _update_counter = _update_frequency;
}

void
LibtorchDRLControlTrainer::getResponseDataFromReporter(
    std::vector<std::vector<std::vector<Real>>> & data_current,
    std::vector<std::vector<std::vector<Real>>> & data_next,
    const std::vector<const std::vector<std::vector<Real>> *> & reporter_links,
    const unsigned int num_timesteps)
{
  for (const auto & state_i : index_range(reporter_links))
  {
    // Fetch the vector of time series for a given reporter
    const std::vector<std::vector<Real>> & reporter_data = *reporter_links[state_i];

    // Made it to the inner loop which is just the different samples
    for (const auto & start_i : make_range(num_timesteps))
    {
      const auto input_i = start_i*reporter_links.size() + state_i;
      for (const auto & sample : reporter_data)
      {
        const unsigned int sample_vector_size = sample.size() - _shift_outputs;
        const unsigned int num_entries_kept = sample_vector_size / _timestep_window;
        std::vector<Real> split_sample(num_entries_kept, 0.0);
        std::vector<Real> next_split_sample(num_entries_kept, 0.0);

        unsigned int current_real_i = 0;
        unsigned int next_current_real_i = 0;
        for (unsigned int time_i = 0; time_i < sample_vector_size; ++time_i)
        {
          if (!(time_i % _timestep_window))
          {
            if (time_i < start_i)
              split_sample[current_real_i] = sample[0];
            else
            {
              const auto shifted_i = time_i - start_i;
              split_sample[current_real_i] = sample[shifted_i];
            }
            current_real_i++;
          }

          if (!(time_i % _timestep_window) && (time_i + _timestep_window < sample_vector_size + _shift_outputs))
          {
              const auto shifted_i = time_i + _timestep_window - start_i;
              next_split_sample[next_current_real_i] = sample[shifted_i];
              next_current_real_i++;
          }
        }

        data_current[input_i].push_back(std::move(split_sample));
        data_next[input_i].push_back(std::move(next_split_sample));
      }
    }
  }
  // std::cout << " finished " << std::endl;
}

void LibtorchDRLControlTrainer::normalizeResponseData(std::vector<std::vector<std::vector<Real>>> & data,
                                                      const unsigned int num_timesteps)
{
  // std::cout << " Normalizing " << Moose::stringify(data) << std::endl;
  // We have multiple reporters, each has a time series for each sample
  const auto num_reporters = data.size() / num_timesteps;
  for (const auto & rep_i : make_range(num_reporters))
  {
    // We shift and scale the inputs to get better training efficiency
    for (const auto & start_step : make_range(num_timesteps))
    {
      unsigned int real_i = num_reporters * start_step + rep_i;

      for (const auto sample_i : index_range(data[real_i]))
      {
        std::transform(
            data[real_i][sample_i].begin(),
            data[real_i][sample_i].end(),
            data[real_i][sample_i].begin(),
            [this, &rep_i](Real value) -> Real
            { return (value - _state_shift_factors[rep_i]) * _state_scaling_factors[rep_i]; });
      }
    }
  }
}

void
LibtorchDRLControlTrainer::getSignalDataFromReporter(
    std::vector<std::vector<std::vector<Real>>> & data,
    const std::vector<const std::vector<std::vector<Real>> *> & reporter_links)
{
  for (const auto & action_i : index_range(reporter_links))
  {
    // Fetch the vector of time series for a given reporter
    const std::vector<std::vector<Real>> & reporter_data = *reporter_links[action_i];

    for (const auto & sample : reporter_data)
    {
      const unsigned int sample_vector_size = sample.size() - _shift_outputs;
      const unsigned int num_entries_kept = sample_vector_size / _timestep_window;
      std::vector<Real> action_for_sample(num_entries_kept, 0.0);

      unsigned int real_i = 0;
      for (const auto time_i : make_range(sample_vector_size))
        if (!(time_i % _timestep_window))
        {
          action_for_sample[real_i] = sample[time_i + _shift_outputs];
          real_i++;
        }

      data[action_i].push_back(std::move(action_for_sample));
    }
  }
}

void
LibtorchDRLControlTrainer::getRewardDataFromReporter(std::vector<std::vector<Real>> & data,
                                                     const std::vector<std::vector<Real>> * const reporter_link)
{
  // Fetch the vector of time series for a given reporter
  const std::vector<std::vector<Real>> & reporter_data = *reporter_link;

  for (const auto & sample : reporter_data)
  {
    const unsigned int sample_vector_size = sample.size() - _shift_outputs;
    const unsigned int num_entries_kept = sample_vector_size / _timestep_window;

    std::vector<Real> reward_for_sample(num_entries_kept, 0.0);

    unsigned int real_i = 0;
    for (const auto time_i : make_range(sample_vector_size))
      if (!(time_i % _timestep_window) && (time_i + _timestep_window < sample_vector_size + _shift_outputs))
      {
        reward_for_sample[real_i] = sample[time_i + _timestep_window];
        real_i++;
      }

    data.push_back(std::move(reward_for_sample));
  }

  // Fill the corresponding container
  // for (const auto sample_i : index_range(*reporter_link))
  // {
  //   for (const unsigned int state_i = _shift_outputs; state_i <  (*reporter_link)[sample_i].size(); state_i++)
  //       {
  //         if (!((state_i - _shift_outputs) % _timestep_window))
  //           data.push_back((*reporter_link)[sample_i][state_i]);
  //       }
  // }
    // data.insert(data.end(), (*reporter_link)[sample_i].begin() + _shift_outputs, (*reporter_link)[sample_i].end());
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
