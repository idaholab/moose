//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchDRLControl.h"
#include "TorchScriptModule.h"
#include "Transient.h"
#include "LibtorchUtils.h"

registerMooseObject("StochasticToolsApp", LibtorchDRLControl);

InputParameters
LibtorchDRLControl::validParams()
{
  InputParameters params = LibtorchNeuralNetControl::validParams();
  params.addClassDescription(
      "Sets the value of multiple 'Real' input parameters and postprocessors based on a Deep "
      "Reinforcement Learning (DRL) neural network trained using a PPO algorithm.");

  params.addParam<unsigned int>("seed", "Seed for the random number generator.");

  params.addParam<unsigned int>("num_stems_in_period", 1, "Blabla");
  params.addParam<Real>("smoother", 1.0, "Blabla");

  params.addParam<bool>("stochastic", true, "Blabla");

  params.addParam<std::vector<Real>>("min_control_value", {}, "The minimum values of the control signal.");
  params.addParam<std::vector<Real>>("max_control_value", {}, "The maximum calue of the control signal.");
  params.addParam<std::vector<Real>>(
      "action_standard_deviations",
      {},
      "Deprecated compatibility parameter. Actor policies now learn their own action "
      "distribution widths.");

  return params;
}

LibtorchDRLControl::LibtorchDRLControl(const InputParameters & parameters)
  : LibtorchNeuralNetControl(parameters),
    _current_control_signal_log_probabilities(std::vector<Real>(_control_names.size(), 0.0)),
    _previous_control_signal(std::vector<Real>(_control_names.size(), 0.0)),
    _current_smoothed_signal(std::vector<Real>(_control_names.size(), 0.0)),
    _call_counter(0),
    _num_steps_in_period(getParam<unsigned int>("num_stems_in_period")),
    _smoother(getParam<Real>("smoother")),
    _stochastic(getParam<bool>("stochastic"))
{
  // Fixing the RNG seed to make sure every experiment is the same.
  if (isParamValid("seed"))
    torch::manual_seed(getParam<unsigned int>("seed"));

  if (isParamValid("filename"))
    loadControlNeuralNetFromFile(parameters);
}

void
LibtorchDRLControl::loadControlNeuralNetFromFile(const InputParameters & parameters)
{
  const auto & filename = getParam<std::string>("filename");
  if (getParam<bool>("torch_script_format"))
  {
    _actor_nn.reset();
    _nn = std::make_shared<Moose::TorchScriptModule>(filename);
  }
  else
  {
    unsigned int num_inputs = _response_names.size() * _input_timesteps;
    unsigned int num_outputs = _control_names.size();
    std::vector<unsigned int> num_neurons_per_layer =
        getParam<std::vector<unsigned int>>("num_neurons_per_layer");
    std::vector<std::string> activation_functions =
        parameters.isParamSetByUser("activation_function")
            ? getParam<std::vector<std::string>>("activation_function")
            : std::vector<std::string>({"relu"});

    const std::vector<Real> & minimum_values = getParam<std::vector<Real>>("min_control_value");
    const std::vector<Real> & maximum_values = getParam<std::vector<Real>>("max_control_value");

    auto nn = std::make_shared<Moose::LibtorchActorNeuralNet>(
        filename, num_inputs, num_outputs, num_neurons_per_layer, activation_functions, minimum_values, maximum_values);

    try
    {
      torch::load(nn, filename);
      _actor_nn = std::make_shared<Moose::LibtorchActorNeuralNet>(*nn);
      _nn = _actor_nn;
    }
    catch (const c10::Error & e)
    {
      mooseError(
          "The requested pytorch parameter file could not be loaded. This can either be the"
          "result of the file not existing or a misalignment in the generated container and"
          "the data in the file. Make sure the dimensions of the generated neural net are the"
          "same as the dimensions of the parameters in the input file!\n",
          e.msg());
    }
  }
}

void
LibtorchDRLControl::execute()
{
  if (!_actor_nn && !_nn)
    return;

  if (_current_execute_flag != EXEC_TIMESTEP_BEGIN)
    return;

  const unsigned int n_controls = _control_names.size();
  const unsigned int num_old_timesteps = _input_timesteps - 1;

  // Fill a vector with the current values of the responses.
  updateCurrentResponse();

  // Seed the response history with the initial response when the control first runs.
  if (_old_responses.empty())
    _old_responses.assign(num_old_timesteps, _current_response);

  if (_call_counter % _num_steps_in_period == 0)
  {
    torch::Tensor input_tensor = prepareInputTensor();
    torch::Tensor action;

    if (_actor_nn)
    {
      action = _actor_nn->evaluate(input_tensor, _stochastic);

      if (_stochastic)
      {
        torch::Tensor log_probability = _actor_nn->logProbability(action);
        _current_control_signal_log_probabilities = {log_probability.data_ptr<Real>(),
                                                     log_probability.data_ptr<Real>() +
                                                         log_probability.size(1)};
      }
      else
        _current_control_signal_log_probabilities.assign(n_controls, 0.0);
    }
    else
    {
      action = _nn->forward(input_tensor);
      _current_control_signal_log_probabilities.assign(n_controls, 0.0);
    }

    _current_control_signals = {action.data_ptr<Real>(), action.data_ptr<Real>() + action.size(1)};

    if (_call_counter == 0)
      _current_smoothed_signal = _current_control_signals;
  }

  _previous_control_signal = _current_smoothed_signal;

  for (const auto i : index_range(_current_smoothed_signal))
    _current_smoothed_signal[i] =
        _previous_control_signal[i] +
        _smoother * (_current_control_signals[i] - _previous_control_signal[i]);

  for (unsigned int control_i = 0; control_i < n_controls; ++control_i)
    setControllableValueByName<Real>(_control_names[control_i],
                                     _current_smoothed_signal[control_i] *
                                         _action_scaling_factors[control_i]);

  if (_old_responses.size())
  {
    std::rotate(_old_responses.rbegin(), _old_responses.rbegin() + 1, _old_responses.rend());
    _old_responses[0] = _current_response;
  }

  _call_counter++;
}

void
LibtorchDRLControl::loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn)
{
  const auto * check = dynamic_cast<const Moose::LibtorchActorNeuralNet *>(&input_nn);
  if (!check)
    mooseError("This needs to be a LibtorchActorNeuralNet!");
  _nn = std::make_shared<Moose::LibtorchActorNeuralNet>(*check);
  _actor_nn = std::make_shared<Moose::LibtorchActorNeuralNet>(*check);
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
