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

  return params;
}

LibtorchDRLControl::LibtorchDRLControl(const InputParameters & parameters)
  : LibtorchNeuralNetControl(parameters),
    _current_control_signal_log_probabilities(std::vector<Real>(_control_names.size(), 0.0)),
    _previous_control_signal(std::vector<Real>(_control_names.size(), 0.0)),
    _current_smoothed_signal(std::vector<Real>(_control_names.size(), 0.0)),
    _call_counter(0),
    _num_steps_in_period(getParam<unsigned int>("num_stems_in_period")),
    _smoother(getParam<Real>("smoother"))
{
  // Fixing the RNG seed to make sure every experiment is the same.
  if (isParamValid("seed"))
    torch::manual_seed(getParam<unsigned int>("seed"));
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

    if (_current_execute_flag == EXEC_TIMESTEP_BEGIN)
    {
      // If this is the first time this control is called and we need to use older values, fill up the
      // needed old values using the initial values
      if (!_initialized)
      {
        _old_responses.clear();
        for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
          _old_responses.push_back(_current_response);
        _initialized = true;
      }

      if (_call_counter % _num_steps_in_period == 0)
      {
        // Organize the old an current solution into a tensor so we can evaluate the neural net
        torch::Tensor input_tensor = prepareInputTensor();

        // Evaluate the neural network to get the expected control value
        torch::Tensor action = _actor_nn->forward(input_tensor);

        // Compute log probability
        torch::Tensor log_probability = _actor_nn->logProbability();

        _current_control_signals = {action.data_ptr<Real>(), action.data_ptr<Real>() + action.size(1)};

        // std::cout << "Computing control signal to: " << Moose::stringify(_current_control_signals) << std::endl;


        // for (const auto i : index_range(_current_control_signals))
        //   _current_control_signals[i] = std::min(std::max(_current_control_signals[i], _minimum_actions[i]), _maximum_actions[i]);

        _current_control_signal_log_probabilities = {log_probability.data_ptr<Real>(),
                                                    log_probability.data_ptr<Real>() +
                                                      log_probability.size(1)};
      }


      // Convert data
      _previous_control_signal = _current_smoothed_signal;


      for (const auto i : index_range(_current_smoothed_signal))
        _current_smoothed_signal[i] = _previous_control_signal[i] + _smoother*(_current_control_signals[i] - _previous_control_signal[i]);


      // std::cout << "Setting control signal to: " << Moose::stringify(_current_smoothed_signal) << std::endl;


      for (unsigned int control_i = 0; control_i < n_controls; ++control_i)
      {

        // We scale the controllable value for physically meaningful control action
        setControllableValueByName<Real>(_control_names[control_i],
                                        _current_smoothed_signal[control_i] *
                                            _action_scaling_factors[control_i]);
      }

      // We add the curent solution to the old solutions and move everything in there one step
      // backward
      if (_old_responses.size())
      {
        std::rotate(_old_responses.rbegin(), _old_responses.rbegin() + 1, _old_responses.rend());
        _old_responses[0] = _current_response;
      }
      _call_counter++;
    }
  }
}

void
LibtorchDRLControl::loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn)
{
  const auto * check = dynamic_cast<const Moose::LibtorchActorNeuralNet *>(&input_nn);
  if (!check)
    mooseError("This needs to be a LibtorchActorNeuralNet!");
  _nn = std::make_shared<Moose::LibtorchActorNeuralNet>(*check);
  _actor_nn = dynamic_cast<Moose::LibtorchActorNeuralNet *>(_nn.get());
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
