//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchNeuralNetControl.h"
#include "TorchScriptModule.h"
#include "LibtorchUtils.h"

#include "Transient.h"

registerMooseObject("MooseApp", LibtorchNeuralNetControl);

InputParameters
LibtorchNeuralNetControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Controls the value of multiple controllable input parameters using a "
                             "Libtorch-based neural network.");
  params.addRequiredParam<std::vector<std::string>>(
      "parameters", "Controllable input parameters driven by the network.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "responses", "Postprocessors used as the current observation vector.");
  params.addParam<std::vector<Real>>(
      "response_shift_factors",
      "Optional offsets applied to the observation values before scaling.");
  params.addParam<std::vector<Real>>(
      "response_scaling_factors",
      "Optional multipliers applied after shifting the observation values.");
  params.addParam<std::string>("filename", "Checkpoint file to load for the controller network.");
  params.addParam<bool>("torch_script_format",
                        false,
                        "Whether the checkpoint should be read as a scripted Torch module.");
  params.addParam<unsigned int>(
      "input_timesteps", 1, "Number of recent timesteps to stack into each network input.");
  params.addParam<std::vector<unsigned int>>(
      "num_neurons_per_layer", "Hidden-layer widths used when constructing the controller.");
  params.addParam<std::vector<std::string>>(
      "activation_function",
      std::vector<std::string>({"relu"}),
      "Activation name for each hidden layer, or one shared value for all layers.");

  params.addParam<std::vector<Real>>(
      "action_scaling_factors",
      "Per-action scaling embedded in the controller outputs so saved checkpoints stay in "
      "physical units.");

  return params;
}

LibtorchNeuralNetControl::LibtorchNeuralNetControl(const InputParameters & parameters)
  : Control(parameters),
    _old_responses(declareRestartableData<std::vector<std::vector<Real>>>("old_responses")),
    _control_names(getParam<std::vector<std::string>>("parameters")),
    _current_control_signals(std::vector<Real>(_control_names.size(), 0.0)),
    _response_names(getParam<std::vector<PostprocessorName>>("responses")),
    _input_timesteps(getParam<unsigned int>("input_timesteps")),
    _response_shift_factors(isParamValid("response_shift_factors")
                                ? getParam<std::vector<Real>>("response_shift_factors")
                                : std::vector<Real>(_response_names.size(), 0.0)),
    _response_scaling_factors(isParamValid("response_scaling_factors")
                                  ? getParam<std::vector<Real>>("response_scaling_factors")
                                  : std::vector<Real>(_response_names.size(), 1.0)),
    _action_scaling_factors(isParamValid("action_scaling_factors")
                                ? getParam<std::vector<Real>>("action_scaling_factors")
                                : std::vector<Real>(_control_names.size(), 1.0)),
    _observation_history(_input_timesteps, _response_shift_factors, _response_scaling_factors)
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
}

void
LibtorchNeuralNetControl::initialSetup()
{
  // File-backed controllers are loaded after full construction so derived controls can override
  // the loader without constructor-time type checks.
  if (isParamSetByUser("filename"))
    loadControlNeuralNetFromFile();
}

void
LibtorchNeuralNetControl::loadControlNeuralNetFromFile()
{
  const auto & filename = getParam<std::string>("filename");
  if (getParam<bool>("torch_script_format"))
    _nn = std::make_shared<Moose::TorchScriptModule>(filename);
  else
  {
    unsigned int num_inputs = _response_names.size() * _input_timesteps;
    unsigned int num_outputs = _control_names.size();
    std::vector<unsigned int> num_neurons_per_layer =
        getParam<std::vector<unsigned int>>("num_neurons_per_layer");
    std::vector<std::string> activation_functions =
        isParamSetByUser("activation_function")
            ? getParam<std::vector<std::string>>("activation_function")
            : std::vector<std::string>({"relu"});
    const auto input_shift_factors =
        _observation_history.expandFeatureFactors(_response_shift_factors);
    const auto input_scaling_factors =
        _observation_history.expandFeatureFactors(_response_scaling_factors);
    auto nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(filename,
                                                                   num_inputs,
                                                                   num_outputs,
                                                                   num_neurons_per_layer,
                                                                   activation_functions,
                                                                   torch::kCPU,
                                                                   torch::kDouble,
                                                                   true,
                                                                   input_shift_factors,
                                                                   input_scaling_factors,
                                                                   _action_scaling_factors);

    try
    {
      Moose::loadLibtorchArtificialNeuralNetState(*nn, filename);
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

  execute();
}

void
LibtorchNeuralNetControl::execute()
{
  if (_nn)
  {
    const unsigned int n_controls = _control_names.size();

    // Fetch current reporter values and populate _current_response
    updateCurrentResponse();

    // If this is the first timestep, we fill up the old values with the initial value
    if (_old_responses.empty())
      _observation_history.initializeHistory(_current_response, _old_responses);

    // Organize the old an current solution into a tensor so we can evaluate the neural net
    torch::Tensor input_tensor = prepareInputTensor();

    // Evaluate the neural network to get the control values then convert it back to vectors
    torch::Tensor action = _nn->forward(input_tensor);

    _current_control_signals = {action.data_ptr<Real>(), action.data_ptr<Real>() + action.size(1)};
    for (unsigned int control_i = 0; control_i < n_controls; ++control_i)
    {
      setControllableValueByName<Real>(_control_names[control_i],
                                       _current_control_signals[control_i]);
    }

    // We add the curent solution to the old solutions and move everything in there one step
    // backward
    if (_old_responses.size())
      _observation_history.advanceHistory(_current_response, _old_responses);
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
  std::vector<Real> raw_response;
  raw_response.reserve(_response_names.size());
  for (const auto & resp_i : index_range(_response_names))
    raw_response.push_back(*_response_values[resp_i]);

  _current_response = raw_response;
}

void
LibtorchNeuralNetControl::loadControlNeuralNet(const Moose::LibtorchArtificialNeuralNet & input_nn)
{
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(input_nn);
}

torch::Tensor
LibtorchNeuralNetControl::prepareInputTensor()
{
  auto raw_input = _observation_history.stackCurrentObservation(_current_response, _old_responses);
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
