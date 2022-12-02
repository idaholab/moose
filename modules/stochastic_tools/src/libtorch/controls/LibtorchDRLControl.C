//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

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
  params.addRequiredParam<std::vector<Real>>(
      "action_standard_deviations", "Standard deviation value used while sampling the actions.");
  params.addParam<unsigned int>("seed", "Seed for the random number generator.");

  return params;
}

LibtorchDRLControl::LibtorchDRLControl(const InputParameters & parameters)
  : LibtorchNeuralNetControl(parameters),
    _current_control_signal_log_probabilities(std::vector<Real>(_control_names.size(), 0.0)),
    _action_std(getParam<std::vector<Real>>("action_standard_deviations"))
{
  if (_control_names.size() != _action_std.size())
    paramError("action_standard_deviations",
               "Number of action_standard_deviations does not match the number of controlled "
               "parameters.");

  // Fixing the RNG seed to make sure every experiment is the same.
  if (isParamValid("seed"))
    torch::manual_seed(getParam<unsigned int>("seed"));

  // We convert and store the user-supplied standard deviations into a tensor which can be easily
  // used by routines in libtorch
  _std = torch::eye(_control_names.size());
  for (unsigned int i = 0; i < _control_names.size(); ++i)
    _std[i][i] = _action_std[i];
}

void
LibtorchDRLControl::execute()
{
  if (_nn)
  {
    unsigned int n_controls = _control_names.size();
    unsigned int num_old_timesteps = _input_timesteps - 1;

    // Fill a vector with the current values of the responses
    updateCurrentResponse();

    // If this is the first time this control is called and we need to use older values, fill up the
    // needed old values using the initial values
    if (!_initialized)
    {
      _old_responses.clear();
      for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
        _old_responses.push_back(_current_response);
      _initialized = true;
    }

    // Organize the old an current solution into a tensor so we can evaluate the neural net
    torch::Tensor input_tensor = prepareInputTensor();

    // Evaluate the neural network to get the expected control value
    torch::Tensor output_tensor = _nn->forward(input_tensor);

    // Sample control value (action) from Gaussian distribution
    torch::Tensor action = at::normal(output_tensor, _std);

    // Compute log probability
    torch::Tensor log_probability = computeLogProbability(action, output_tensor);

    // Convert data
    _current_control_signals = {action.data_ptr<Real>(), action.data_ptr<Real>() + action.size(1)};

    _current_control_signal_log_probabilities = {log_probability.data_ptr<Real>(),
                                                 log_probability.data_ptr<Real>() +
                                                     log_probability.size(1)};

    for (unsigned int control_i = 0; control_i < n_controls; ++control_i)
    {
      // We scale the controllable value for physically meaningful control action
      setControllableValueByName<Real>(_control_names[control_i],
                                       _current_control_signals[control_i] *
                                           _action_scaling_factors[control_i]);
    }

    // We add the curent solution to the old solutions and move everything in there one step
    // backward
    std::rotate(_old_responses.rbegin(), _old_responses.rbegin() + 1, _old_responses.rend());
    _old_responses[0] = _current_response;
  }
}

torch::Tensor
LibtorchDRLControl::computeLogProbability(const torch::Tensor & action,
                                          const torch::Tensor & output_tensor)
{
  // Logarithmic probability of taken action, given the current distribution.
  torch::Tensor var = torch::matmul(_std, _std);

  return -((action - output_tensor) * (action - output_tensor)) / (2.0 * var) - torch::log(_std) -
         std::log(std::sqrt(2.0 * M_PI));
}

Real
LibtorchDRLControl::getSignalLogProbability(const unsigned int signal_index) const
{
  mooseAssert(signal_index < _control_names.size(),
              "The index of the requested control signal is not in the [0," +
                  std::to_string(_control_names.size()) + ") range!");
  return _current_control_signal_log_probabilities[signal_index];
}

#endif
