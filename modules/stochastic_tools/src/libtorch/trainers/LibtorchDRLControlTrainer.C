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
#include "LibtorchArtificialNeuralNetTrainer.h"
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
  return params;
}

LibtorchDRLControlTrainer::LibtorchDRLControlTrainer(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    _response_names(getParam<std::vector<ReporterName>>("response")),
    _response_shift_factors(isParamValid("response_shift_factors")
                                ? getParam<std::vector<Real>>("response_shift_factors")
                                : std::vector<Real>(_response_names.size(), 0.0)),
    _response_scaling_factors(isParamValid("response_scaling_factors")
                                  ? getParam<std::vector<Real>>("response_scaling_factors")
                                  : std::vector<Real>(_response_names.size(), 1.0)),
    _control_names(getParam<std::vector<ReporterName>>("control")),
    _log_probability_names(getParam<std::vector<ReporterName>>("log_probability")),
    _reward_name(getParam<ReporterName>("reward")),
    _reward_value_pointer(&getReporterValueByName<std::vector<Real>>(_reward_name)),
    _input_timesteps(getParam<unsigned int>("input_timesteps")),
    _num_inputs(_input_timesteps * _response_names.size()),
    _num_outputs(_control_names.size()),
    _input_data(std::vector<std::vector<Real>>(_num_inputs)),
    _output_data(std::vector<std::vector<Real>>(_num_outputs)),
    _log_probability_data(std::vector<std::vector<Real>>(_num_outputs)),
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
    _action_std(getParam<std::vector<Real>>("action_standard_deviations")),
    _filename_base(isParamValid("filename_base") ? getParam<std::string>("filename_base") : ""),
    _read_from_file(getParam<bool>("read_from_file")),
    _shift_outputs(getParam<bool>("shift_outputs")),
    _standardize_advantage(getParam<bool>("standardize_advantage")),
    _loss_print_frequency(getParam<unsigned int>("loss_print_frequency")),
    _update_counter(_update_frequency)
{
  if (_response_names.size() != _response_shift_factors.size())
    paramError("response_shift_factors",
               "The number of shift factors is not the same as the number of responses!");

  if (_response_names.size() != _response_scaling_factors.size())
    paramError(
        "response_scaling_factors",
        "The number of normalization coefficients is not the same as the number of responses!");

  // We establish the links with the chosen reporters
  getReporterPointers(_response_names, _response_value_pointers);
  getReporterPointers(_control_names, _control_value_pointers);
  getReporterPointers(_log_probability_names, _log_probability_value_pointers);

  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(getParam<unsigned int>("seed"));

  // Convert the user input standard deviations to a diagonal tensor
  _std = torch::eye(_control_names.size());
  for (unsigned int i = 0; i < _control_names.size(); ++i)
    _std[i][i] = _action_std[i];

  bool filename_valid = isParamValid("filename_base");

  // Initializing the control neural net so that the control can grab it right away
  _control_nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      filename_valid ? _filename_base + "_control.net" : "control.net",
      _num_inputs,
      _num_outputs,
      _num_control_neurons_per_layer,
      getParam<std::vector<std::string>>("control_activation_functions"));

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
}

void
LibtorchDRLControlTrainer::execute()
{
  // Extract data from the reporters
  getInputDataFromReporter(_input_data, _response_value_pointers, _input_timesteps);
  getOutputDataFromReporter(_output_data, _control_value_pointers);
  getOutputDataFromReporter(_log_probability_data, _log_probability_value_pointers);
  getRewardDataFromReporter(_reward_data, _reward_value_pointer);

  // Calculate return from the reward (discounting the reward)
  computeRewardToGo();

  _update_counter--;

  // Only update the NNs when
  if (_update_counter == 0)
  {
    // We compute the average reward first
    computeAverageEpisodeReward();

    // Transform input/output/return data to torch::Tensor
    convertDataToTensor(_input_data, _input_tensor);
    convertDataToTensor(_output_data, _output_tensor);
    convertDataToTensor(_log_probability_data, _log_probability_tensor);

    // Discard (detach) the gradient info for return data
    LibtorchUtils::vectorToTensor<Real>(_return_data, _return_tensor, true);

    // We train the controller using the emulator to get a good control strategy
    trainController();

    // We clean the training data after contoller update and reset the counter
    resetData();
  }
}

void
LibtorchDRLControlTrainer::computeAverageEpisodeReward()
{
  if (_reward_data.size())
    _average_episode_reward =
        std::accumulate(_reward_data.begin(), _reward_data.end(), 0.0) / _reward_data.size();
  else
    _average_episode_reward = 0.0;
}

void
LibtorchDRLControlTrainer::computeRewardToGo()
{
  // Get reward data from one simulation
  std::vector<Real> reward_data_per_sim;
  std::vector<Real> return_data_per_sim;
  getRewardDataFromReporter(reward_data_per_sim, _reward_value_pointer);

  // Discount the reward to get the return value, we need this to be able to anticipate
  // rewards based on the current behavior.
  Real discounted_reward(0.0);
  for (int i = reward_data_per_sim.size() - 1; i >= 0; --i)
  {
    discounted_reward = reward_data_per_sim[i] + discounted_reward * _decay_factor;

    // We are inserting to the front of the vector and push the rest back, this will
    // ensure that the first element of the vector is the discounter reward for the whole transient
    return_data_per_sim.insert(return_data_per_sim.begin(), discounted_reward);
  }

  // Save and accumulate the return values
  _return_data.insert(_return_data.end(), return_data_per_sim.begin(), return_data_per_sim.end());
}

void
LibtorchDRLControlTrainer::trainController()
{
  // Define the optimizers for the training
  torch::optim::Adam actor_optimizer(_control_nn->parameters(),
                                     torch::optim::AdamOptions(_control_learning_rate));

  torch::optim::Adam critic_optimizer(_critic_nn->parameters(),
                                      torch::optim::AdamOptions(_critic_learning_rate));

  // Compute the approximate value (return) from the critic neural net and use it to compute an
  // advantage
  auto value = evaluateValue(_input_tensor).detach();
  auto advantage = _return_tensor - value;

  // If requested, standardize the advantage
  if (_standardize_advantage)
    advantage = (advantage - advantage.mean()) / (advantage.std() + 1e-10);

  for (unsigned int epoch = 0; epoch < _num_epochs; ++epoch)
  {
    // Get the approximate return from the neural net again (this one does have an associated
    // gradient)
    value = evaluateValue(_input_tensor);
    // Get the approximate logarithmic action probability using the control neural net
    auto curr_log_probability = evaluateAction(_input_tensor, _output_tensor);

    // Prepare the ratio by using the e^(logx-logy)=x/y expression
    auto ratio = (curr_log_probability - _log_probability_tensor).exp();

    // Use clamping for limiting
    auto surr1 = ratio * advantage;
    auto surr2 = torch::clamp(ratio, 1.0 - _clip_param, 1.0 + _clip_param) * advantage;

    // Compute loss values for the critic and the control neural net
    auto actor_loss = -torch::min(surr1, surr2).mean();
    auto critic_loss = torch::mse_loss(value, _return_tensor);

    // Update the weights in the neural nets
    actor_optimizer.zero_grad();
    actor_loss.backward();
    actor_optimizer.step();

    critic_optimizer.zero_grad();
    critic_loss.backward();
    critic_optimizer.step();

    // print loss per epoch
    if (_loss_print_frequency)
      if (epoch % _loss_print_frequency == 0)
        _console << "Epoch: " << epoch << " | Actor Loss: " << COLOR_GREEN
                 << actor_loss.item<double>() << COLOR_DEFAULT << " | Critic Loss: " << COLOR_GREEN
                 << critic_loss.item<double>() << COLOR_DEFAULT << std::endl;
  }

  // Save the controller neural net so our controller can read it, we also save the critic if we
  // want to continue training
  torch::save(_control_nn, _control_nn->name());
  torch::save(_critic_nn, _critic_nn->name());
}

void
LibtorchDRLControlTrainer::convertDataToTensor(std::vector<std::vector<Real>> & vector_data,
                                               torch::Tensor & tensor_data,
                                               const bool detach)
{
  for (unsigned int i = 0; i < vector_data.size(); ++i)
  {
    torch::Tensor input_row;
    LibtorchUtils::vectorToTensor(vector_data[i], input_row, detach);

    if (i == 0)
      tensor_data = input_row;
    else
      tensor_data = torch::cat({tensor_data, input_row}, 1);
  }

  if (detach)
    tensor_data.detach();
}

torch::Tensor
LibtorchDRLControlTrainer::evaluateValue(torch::Tensor & input)
{
  return _critic_nn->forward(input);
}

torch::Tensor
LibtorchDRLControlTrainer::evaluateAction(torch::Tensor & input, torch::Tensor & output)
{
  torch::Tensor var = torch::matmul(_std, _std);

  // Compute an action and get it's logarithmic proability based on an assumed Gaussian distribution
  torch::Tensor action = _control_nn->forward(input);
  return -((action - output) * (action - output)) / (2 * var) - torch::log(_std) -
         std::log(std::sqrt(2 * M_PI));
}

void
LibtorchDRLControlTrainer::resetData()
{
  for (auto & data : _input_data)
    data.clear();
  for (auto & data : _output_data)
    data.clear();
  for (auto & data : _log_probability_data)
    data.clear();

  _reward_data.clear();
  _return_data.clear();

  _update_counter = _update_frequency;
}

void
LibtorchDRLControlTrainer::getInputDataFromReporter(
    std::vector<std::vector<Real>> & data,
    const std::vector<const std::vector<Real> *> & reporter_links,
    const unsigned int num_timesteps)
{
  for (const auto & rep_i : index_range(reporter_links))
  {
    std::vector<Real> reporter_data = *reporter_links[rep_i];

    // We shift and scale the inputs to get better training efficiency
    std::transform(
        reporter_data.begin(),
        reporter_data.end(),
        reporter_data.begin(),
        [this, &rep_i](Real value) -> Real
        { return (value - _response_shift_factors[rep_i]) * _response_scaling_factors[rep_i]; });

    // Fill the corresponding containers
    for (const auto & start_step : make_range(num_timesteps))
    {
      unsigned int row = reporter_links.size() * start_step + rep_i;
      for (unsigned int fill_i = 1; fill_i < num_timesteps - start_step; ++fill_i)
        data[row].push_back(reporter_data[0]);

      data[row].insert(data[row].end(),
                       reporter_data.begin(),
                       reporter_data.begin() + start_step + reporter_data.size() -
                           (num_timesteps - 1) - _shift_outputs);
    }
  }
}

void
LibtorchDRLControlTrainer::getOutputDataFromReporter(
    std::vector<std::vector<Real>> & data,
    const std::vector<const std::vector<Real> *> & reporter_links)
{
  for (const auto & rep_i : index_range(reporter_links))
    // Fill the corresponding containers
    data[rep_i].insert(data[rep_i].end(),
                       reporter_links[rep_i]->begin() + _shift_outputs,
                       reporter_links[rep_i]->end());
}

void
LibtorchDRLControlTrainer::getRewardDataFromReporter(std::vector<Real> & data,
                                                     const std::vector<Real> * const reporter_link)
{
  // Fill the corresponding container
  data.insert(data.end(), reporter_link->begin() + _shift_outputs, reporter_link->end());
}

void
LibtorchDRLControlTrainer::getReporterPointers(
    const std::vector<ReporterName> & reporter_names,
    std::vector<const std::vector<Real> *> & pointer_storage)
{
  pointer_storage.clear();
  for (const auto & name : reporter_names)
    pointer_storage.push_back(&getReporterValueByName<std::vector<Real>>(name));
}

#endif
