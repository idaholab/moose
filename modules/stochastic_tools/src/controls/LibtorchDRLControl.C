//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibtorchDRLControl.h"
#include "LibtorchTorchScriptNeuralNet.h"
#include "LibtorchArtificialNeuralNet.h"
#include "Transient.h"

registerMooseObject("StochasticToolsApp", LibtorchDRLControl);

InputParameters
LibtorchDRLControl::validParams()
{
  InputParameters params = LibtorchNeuralNetControl::validParams();
  params.addClassDescription(
      "Sets the value of multiple 'Real' input parameters and postprocessors based on a Deep "
      "Reinforcement Learning (DRL) neural network trained using a PPO algorithm.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "log_probability_postprocessors",
      "The postprocessors which store the log probability of the action/control values.");
  params.addRequiredParam<std::vector<Real>>(
      "action_standard_deviations", "Standard deviation value used while sampling the actions.");

  return params;
}

LibtorchDRLControl::LibtorchDRLControl(const InputParameters & parameters)
  : LibtorchNeuralNetControl(parameters),
    _log_probability_postprocessor_names(
        getParam<std::vector<PostprocessorName>>("log_probability_postprocessors")),
    _action_std(getParam<std::vector<Real>>("action_standard_deviations"))
{
  if (_control_names.size() != _action_std.size())
    paramError("action_standard_deviations",
               "Number of action_standard_deviations does not match the number of controlled "
               "parameters.");

  if (_control_names.size() != _log_probability_postprocessor_names.size())
    paramError("log_probability_postprocessors",
               "Number of log_probability_postprocessors does not match the number of controlled "
               "parameters.");

#ifdef LIBTORCH_ENABLED
  // We convert and store the user-supplied standard deviations into a tensor which can be easily
  // used by routines in libtorch
  _std = torch::eye(_control_names.size());
  for (unsigned int i = 0; i < _control_names.size(); ++i)
    _std[i][i] = _action_std[i];
#endif
}

void
LibtorchDRLControl::execute()
{
#ifdef LIBTORCH_ENABLED
  if (_nn)
  {
    unsigned int n_responses = _response_names.size();
    unsigned int n_controls = _control_names.size();
    unsigned int num_old_timesteps = _input_timesteps - 1;

    // Fill a vector with the current values of the responses
    _current_response.clear();
    for (unsigned int resp_i = 0; resp_i < n_responses; ++resp_i)
      _current_response.push_back(
          (getPostprocessorValueByName(_response_names[resp_i]) - _response_shift_factors[resp_i]) *
          _response_scaling_factors[resp_i]);

    // If this is the first time this control is called and we need to use older values, fill up the
    // needed old values using the initial values
    if (!_initialized)
    {
      _old_responses.clear();
      for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
        _old_responses.push_back(_current_response);
      _initialized = true;
    }

    // Convert the input to a tensor so that we can call the neural net on it
    std::vector<Real> raw_input(_current_response);
    for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
      raw_input.insert(
          raw_input.end(), _old_responses[step_i].begin(), _old_responses[step_i].end());

    auto options = torch::TensorOptions().dtype(at::kDouble);
    torch::Tensor input_tensor =
        torch::from_blob(raw_input.data(), {1, _input_timesteps * n_responses}, options)
            .to(at::kDouble);

    torch::Tensor output_tensor = _nn->forward(input_tensor);

    // Sample control value (action) from Gaussian distribution
    torch::Tensor action = at::normal(output_tensor, _std);

    // Compute log probability
    torch::Tensor log_probability = computeLogProbability(action, output_tensor);

    // Convert data
    std::vector<Real> converted_action = {action.data_ptr<Real>(),
                                          action.data_ptr<Real>() + action.size(1)};

    std::vector<Real> converted_log_probability = {log_probability.data_ptr<Real>(),
                                                   log_probability.data_ptr<Real>() +
                                                       log_probability.size(1)};

    for (unsigned int control_i = 0; control_i < n_controls; ++control_i)
    {
      // We scale the controllable value for physically meaningful control action
      setControllableValueByName<Real>(_control_names[control_i],
                                       converted_action[control_i] *
                                           _action_scaling_factors[control_i]);
      // Save action values to postprocessor. We do not scale the action value here, it will be used
      // and reported directly for training
      _fe_problem.setPostprocessorValueByName(_action_postprocessor_names[control_i],
                                              converted_action[control_i]);

      // Save log probability values to postprocessor
      _fe_problem.setPostprocessorValueByName(_log_probability_postprocessor_names[control_i],
                                              converted_log_probability[control_i]);
    }

    // We add the curent solution to the old solutions and move everything in there one step
    // backward
    for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
    {
      if (step_i == num_old_timesteps - 1)
        _old_responses[0] = _current_response;
      else
        _old_responses[(num_old_timesteps - 1) - step_i] =
            _old_responses[(num_old_timesteps - 1) - step_i - 1];
    }
  }
#endif
}

#ifdef LIBTORCH_ENABLED
torch::Tensor
LibtorchDRLControl::computeLogProbability(const torch::Tensor & action,
                                          const torch::Tensor & output_tensor)
{
  // Logarithmic probability of taken action, given the current distribution.
  torch::Tensor var = torch::matmul(_std, _std);

  return -((action - output_tensor) * (action - output_tensor)) / (2.0 * var) - torch::log(_std) -
         std::log(std::sqrt(2.0 * M_PI));
}
#endif
