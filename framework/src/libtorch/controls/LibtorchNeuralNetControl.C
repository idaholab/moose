//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchNeuralNetControl.h"
#include "LibtorchTorchScriptNeuralNet.h"
#include "LibtorchUtils.h"

#include "Transient.h"

registerMooseObject("MooseApp", LibtorchNeuralNetControl);

InputParameters
LibtorchNeuralNetControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Controls the value of multiple controllable input parameters using a "
                             "Libtorch-based neural network.");
  params.addRequiredParam<std::vector<std::string>>("parameters",
                                                    "The input parameter(s) to control.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "responses", "The responses (prostprocessors) which are used for the control.");
  params.addParam<std::vector<Real>>(
      "response_shift_factors",
      "Constants which will be used to shift the response values. This is used for the "
      "manipulation of the neural net inputs for better training efficiency.");
  params.addParam<std::vector<Real>>(
      "response_scaling_factors",
      "Constants which will be used to multiply the shifted response values. This is used for "
      "the manipulation of the neural net inputs for better training efficiency.");
  params.addParam<std::string>("filename",
                               "Define if the neural net is supposed to be loaded from a file.");
  params.addParam<bool>("torch_script_format",
                        false,
                        "If we want to load the neural net using the torch-script format.");
  params.addParam<unsigned int>(
      "input_timesteps",
      1,
      "Number of time steps to use in the input data, if larger than 1, "
      "data from the previous timesteps will be used as well as inputs in the training.");
  params.addParam<std::vector<unsigned int>>("num_neurons_per_layer",
                                             "The number of neurons on each hidden layer.");
  params.addParam<std::vector<std::string>>(
      "activation_function",
      std::vector<std::string>({"relu"}),
      "The type of activation functions to use. It is either one value "
      "or one value per hidden layer.");

  params.addParam<std::vector<Real>>(
      "action_scaling_factors",
      "Scale factor that multiplies the NN output to obtain a physically meaningful value.");

  return params;
}

LibtorchNeuralNetControl::LibtorchNeuralNetControl(const InputParameters & parameters)
  : Control(parameters),
    _control_names(getParam<std::vector<std::string>>("parameters")),
    _current_control_signals(std::vector<Real>(_control_names.size(), 0.0)),
    _response_names(getParam<std::vector<PostprocessorName>>("responses")),
    _input_timesteps(getParam<unsigned int>("input_timesteps")),
    _initialized(false),
    _response_shift_factors(isParamValid("response_shift_factors")
                                ? getParam<std::vector<Real>>("response_shift_factors")
                                : std::vector<Real>(_response_names.size(), 0.0)),
    _response_scaling_factors(isParamValid("response_scaling_factors")
                                  ? getParam<std::vector<Real>>("response_scaling_factors")
                                  : std::vector<Real>(_response_names.size(), 1.0)),
    _action_scaling_factors(isParamValid("action_scaling_factors")
                                ? getParam<std::vector<Real>>("action_scaling_factors")
                                : std::vector<Real>(_control_names.size(), 1.0))
{
  // We first check if the input parameters make sense and throw errors if different parameter
  // combinations are not allowed
  conditionalParameterError("filename",
                            {"num_neurons_per_layer", "activation_function"},
                            !getParam<bool>("torch_script_format"));

  if (_response_names.size() != _response_shift_factors.size())
    paramError("response_shift_factors",
               "The number of shift factors is not the same as the number of responses!");

  if (_response_names.size() != _response_scaling_factors.size())
    paramError(
        "response_scaling_factors",
        "The number of normalization coefficients is not the same as the number of responses!");

  if (_control_names.size() != _action_scaling_factors.size())
    paramError("action_scaling_factors",
               "The number of normalization coefficients is not the same as the number of "
               "controlled parameters!");

  // We link to the postprocessor values so that we can fetch them any time. This also raises
  // errors if we don't have the postprocessors requested in the input.
  for (unsigned int resp_i = 0; resp_i < _response_names.size(); ++resp_i)
    _response_values.push_back(&getPostprocessorValueByName(_response_names[resp_i]));

  // If the user wants to read the neural net from file, we do it. We can read it from a
  // torchscript file, or we can create a shell and read back the parameters
  if (isParamValid("filename"))
  {
    std::string filename = getParam<std::string>("filename");
    if (getParam<bool>("torch_script_format"))
      _nn = std::make_shared<Moose::LibtorchTorchScriptNeuralNet>(filename);
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
      auto nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
          filename, num_inputs, num_outputs, num_neurons_per_layer, activation_functions);

      try
      {
        torch::load(nn, filename);
        _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(*nn);
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
}

void
LibtorchNeuralNetControl::execute()
{
  if (_nn)
  {
    const unsigned int n_controls = _control_names.size();
    const unsigned int num_old_timesteps = _input_timesteps - 1;

    // Fetch current reporter values and populate _current_response
    updateCurrentResponse();

    // If this is the first timestep, we fill up the old values with the initial value
    if (!_initialized)
    {
      _old_responses.assign(num_old_timesteps, _current_response);
      _initialized = true;
    }

    // Organize the old an current solution into a tensor so we can evaluate the neural net
    torch::Tensor input_tensor = prepareInputTensor();

    // Evaluate the neural network to get the control values then convert it back to vectors
    torch::Tensor action = _nn->forward(input_tensor);

    _current_control_signals = {action.data_ptr<Real>(), action.data_ptr<Real>() + action.size(1)};
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

Real
LibtorchNeuralNetControl::getSignal(const unsigned int signal_index) const
{
  mooseAssert(signal_index < _control_names.size(),
              "The index of the requested control signal is not in the [0," +
                  std::to_string(_control_names.size()) + ") range!");
  return _current_control_signals[signal_index];
}

void
LibtorchNeuralNetControl::conditionalParameterError(
    const std::string & param_name,
    const std::vector<std::string> & conditional_params,
    bool should_be_defined)
{
  if (parameters().isParamSetByUser(param_name))
    for (const auto & param : conditional_params)
      if (parameters().isParamSetByUser(param) != should_be_defined)
        paramError(param,
                   "This parameter should",
                   (should_be_defined ? " " : " not "),
                   "be defined when ",
                   param_name,
                   " is defined!");
}

void
LibtorchNeuralNetControl::updateCurrentResponse()
{
  // Gather the current response values from the reporters
  _current_response.clear();

  for (const auto & resp_i : index_range(_response_names))
    _current_response.push_back((*_response_values[resp_i] - _response_shift_factors[resp_i]) *
                                _response_scaling_factors[resp_i]);
}

void
LibtorchNeuralNetControl::loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn)
{
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(input_nn);
}

torch::Tensor
LibtorchNeuralNetControl::prepareInputTensor()
{
  const unsigned int num_old_timesteps = _input_timesteps - 1;

  // We convert the standard vectors to libtorch tensors
  std::vector<Real> raw_input(_current_response);

  for (const auto & step_i : make_range(num_old_timesteps))
    raw_input.insert(raw_input.end(), _old_responses[step_i].begin(), _old_responses[step_i].end());

  torch::Tensor input_tensor;
  LibtorchUtils::vectorToTensor(raw_input, input_tensor);

  return input_tensor.transpose(0, 1);
}

const Moose::LibtorchNeuralNetBase &
LibtorchNeuralNetControl::controlNeuralNet() const
{
  if (!hasControlNeuralNet())
    mooseError("The neural network in the controller must exist!");
  return *_nn;
}

#endif
