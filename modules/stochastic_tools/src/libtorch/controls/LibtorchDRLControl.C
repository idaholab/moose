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
#include "LibtorchRandomUtils.h"
#include "Transient.h"
#include "LibtorchUtils.h"

#include <mutex>

registerMooseObject("StochasticToolsApp", LibtorchDRLControl);

InputParameters
LibtorchDRLControl::validParams()
{
  InputParameters params = LibtorchNeuralNetControl::validParams();
  params.addClassDescription(
      "Sets the value of multiple 'Real' input parameters and postprocessors based on a Deep "
      "Reinforcement Learning (DRL) neural network trained using a PPO algorithm.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  params.suppressParameter<bool>("torch_script_format");

  params.addParam<unsigned int>("seed", "Seed for the random number generator.");

  params.addRangeCheckedParam<unsigned int>(
      "num_steps_in_period",
      1,
      "1<=num_steps_in_period",
      "Number of controller executions between policy evaluations.");
  params.addParam<Real>(
      "smoother", 1.0, "Relaxation factor applied when smoothing control updates.");

  params.addParam<std::vector<Real>>(
      "control_offsets",
      {},
      "Optional constant offsets added to each policy signal before setting the controlled "
      "parameter. This can be used to train a policy on deviations from a nominal signal.");

  params.addParam<bool>(
      "stochastic",
      true,
      "If true, sample from the policy distribution; otherwise use the deterministic action.");

  params.addParam<std::vector<Real>>(
      "min_control_value", {}, "Optional lower bounds for each control signal.");
  params.addParam<std::vector<Real>>(
      "max_control_value", {}, "Optional upper bounds for each control signal.");
  params.addParam<bool>(
      "state_independent_std",
      true,
      "If true, interpret the unbounded Gaussian actor as learning one log-std per action "
      "dimension. If false, use a state-dependent std head.");

  return params;
}

LibtorchDRLControl::LibtorchDRLControl(const InputParameters & parameters)
  : LibtorchNeuralNetControl(parameters),
    _current_control_signal_log_probabilities(declareRestartableData<std::vector<Real>>(
        "current_control_signal_log_probabilities", std::vector<Real>(_control_names.size(), 0.0))),
    _previous_control_signal(declareRestartableData<std::vector<Real>>(
        "previous_control_signal", std::vector<Real>(_control_names.size(), 0.0))),
    _current_smoothed_signal(declareRestartableData<std::vector<Real>>(
        "current_smoothed_signal", std::vector<Real>(_control_names.size(), 0.0))),
    _control_offsets(isParamSetByUser("control_offsets")
                         ? getParam<std::vector<Real>>("control_offsets")
                         : std::vector<Real>(_control_names.size(), 0.0)),
    _has_control_offsets(isParamSetByUser("control_offsets")),
    _policy_generator(Moose::makeLibtorchCPUGenerator()),
    _policy_generator_state(declareRestartableData<std::vector<std::uint8_t>>(
        "policy_generator_state", std::vector<std::uint8_t>())),
    _executions_until_next_policy_evaluation(
        declareRestartableData<unsigned int>("executions_until_next_policy_evaluation", 0)),
    _num_steps_in_period(getParam<unsigned int>("num_steps_in_period")),
    _smoother(getParam<Real>("smoother")),
    _stochastic(getParam<bool>("stochastic"))
{
  const auto & execute_on = getParam<ExecFlagEnum>("execute_on");
  if (execute_on.size() != 1 || !execute_on.contains(EXEC_TIMESTEP_BEGIN))
    paramError("execute_on", "LibtorchDRLControl only supports 'TIMESTEP_BEGIN' for 'execute_on'.");

  if (isParamValid("seed"))
    setPolicySampleSeed(getParam<unsigned int>("seed"));

  if (_control_offsets.size() != _control_names.size())
    paramError("control_offsets",
               "The number of control offsets must match the number of controlled parameters.");

  savePolicyGeneratorState();
}

void
LibtorchDRLControl::initialSetup()
{
  LibtorchNeuralNetControl::initialSetup();
  restorePolicyGeneratorState();
  savePolicyGeneratorState();
}

void
LibtorchDRLControl::loadControlNeuralNetFromFile()
{
  const auto & filename = getParam<std::string>("filename");
  unsigned int num_inputs = _observation_names.size() * _input_timesteps;
  unsigned int num_outputs = _control_names.size();
  std::vector<unsigned int> num_neurons_per_layer =
      getParam<std::vector<unsigned int>>("num_neurons_per_layer");
  std::vector<std::string> activation_functions =
      isParamSetByUser("activation_function")
          ? getParam<std::vector<std::string>>("activation_function")
          : std::vector<std::string>({"relu"});

  const std::vector<Real> & minimum_values = getParam<std::vector<Real>>("min_control_value");
  const std::vector<Real> & maximum_values = getParam<std::vector<Real>>("max_control_value");
  const auto input_shift_factors =
      _observation_history.expandObservationFactors(_observation_shift_factors);
  const auto input_scaling_factors =
      _observation_history.expandObservationFactors(_observation_scaling_factors);

  _actor_nn =
      std::make_shared<Moose::LibtorchActorNeuralNet>(filename,
                                                      num_inputs,
                                                      num_outputs,
                                                      num_neurons_per_layer,
                                                      activation_functions,
                                                      minimum_values,
                                                      maximum_values,
                                                      torch::kCPU,
                                                      torch::kDouble,
                                                      true,
                                                      input_shift_factors,
                                                      input_scaling_factors,
                                                      _action_scaling_factors,
                                                      getParam<bool>("state_independent_std"));

  Moose::loadLibtorchActorNeuralNetState(*_actor_nn, filename);
  _nn = _actor_nn;
}

void
LibtorchDRLControl::execute()
{
  if (!_actor_nn)
  {
    mooseAssert(!_nn, "LibtorchDRLControl should not store a non-actor controller network.");
    if (_has_control_offsets)
      applyControlSignals();
    return;
  }

  mooseAssert(_current_execute_flag == EXEC_TIMESTEP_BEGIN,
              "LibtorchDRLControl should only execute on TIMESTEP_BEGIN.");

  const unsigned int n_controls = _control_names.size();
  const bool first_control_execution = _old_observations.empty();

  // Fill a vector with the current observation values.
  updateCurrentObservation();

  // Seed the observation history with the initial observation when the control first runs.
  if (first_control_execution)
    _observation_history.initializeHistory(_current_observation, _old_observations);

  if (shouldEvaluatePolicy())
  {
    torch::Tensor input_tensor = prepareInputTensor();
    torch::Tensor action = _actor_nn->evaluate(input_tensor, _stochastic, _policy_generator);
    savePolicyGeneratorState();

    if (_stochastic)
    {
      torch::Tensor log_probability = _actor_nn->logProbability(action);
      _current_control_signal_log_probabilities = {log_probability.data_ptr<Real>(),
                                                   log_probability.data_ptr<Real>() +
                                                       log_probability.size(1)};
    }
    else
      _current_control_signal_log_probabilities.assign(n_controls, 0.0);

    _current_control_signals = {action.data_ptr<Real>(), action.data_ptr<Real>() + action.size(1)};

  }

  _previous_control_signal = _current_smoothed_signal;

  for (const auto i : index_range(_current_smoothed_signal))
    _current_smoothed_signal[i] =
        _previous_control_signal[i] +
        _smoother * (_current_control_signals[i] - _previous_control_signal[i]);

  applyControlSignals();

  _observation_history.advanceHistory(_current_observation, _old_observations);
}

Real
LibtorchDRLControl::computeControlOffset(const unsigned int control_i) const
{
  return _control_offsets[control_i];
}

void
LibtorchDRLControl::applyControlSignals()
{
  for (const auto control_i : index_range(_control_names))
  {
    const Real applied_signal = computeControlOffset(control_i) + _current_smoothed_signal[control_i];
    setControllableValueByName<Real>(_control_names[control_i], applied_signal);
  }
}

void
LibtorchDRLControl::loadControlNeuralNet(const Moose::LibtorchActorNeuralNet & input_nn)
{
  _actor_nn = std::make_shared<Moose::LibtorchActorNeuralNet>(input_nn);
  _nn = _actor_nn;
}

void
LibtorchDRLControl::loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn)
{
  const auto * check = dynamic_cast<const Moose::LibtorchActorNeuralNet *>(&input_nn);
  if (!check)
    mooseError("This needs to be a LibtorchActorNeuralNet!");
  loadControlNeuralNet(*check);
}

void
LibtorchDRLControl::setPolicySampleSeed(const uint64_t seed)
{
  {
    std::lock_guard<std::mutex> lock(_policy_generator.mutex());
    _policy_generator.set_current_seed(seed);
  }
  savePolicyGeneratorState();
}

bool
LibtorchDRLControl::shouldEvaluatePolicy()
{
  if (_executions_until_next_policy_evaluation == 0)
  {
    _executions_until_next_policy_evaluation = _num_steps_in_period - 1;
    return true;
  }

  --_executions_until_next_policy_evaluation;
  return false;
}

void
LibtorchDRLControl::restorePolicyGeneratorState()
{
  if (!_stochastic || _policy_generator_state.empty())
    return;

  auto state_tensor =
      torch::from_blob(_policy_generator_state.data(),
                       {static_cast<long>(_policy_generator_state.size())},
                       torch::TensorOptions().dtype(torch::kUInt8).device(torch::kCPU))
          .clone();
  std::lock_guard<std::mutex> lock(_policy_generator.mutex());
  _policy_generator.set_state(state_tensor);
}

void
LibtorchDRLControl::savePolicyGeneratorState()
{
  if (!_stochastic)
  {
    _policy_generator_state.clear();
    return;
  }

  std::lock_guard<std::mutex> lock(_policy_generator.mutex());
  const auto state_tensor = _policy_generator.get_state().contiguous();
  const auto * data = state_tensor.data_ptr<std::uint8_t>();
  _policy_generator_state.assign(data, data + static_cast<std::size_t>(state_tensor.numel()));
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
