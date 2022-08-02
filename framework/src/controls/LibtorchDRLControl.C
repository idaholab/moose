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

registerMooseObject("MooseApp", LibtorchDRLControl);

InputParameters
LibtorchDRLControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription(
      "Sets the value of a 'Real' input parameter (or postprocessor) based on a Proportional "
      "Integral Derivative control of a postprocessor to match a target a target value.");
  params.addRequiredParam<std::vector<std::string>>("parameters",
                                                    "The input parameter(s) to control.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "responses", "The responses (prostprocessors) which are used for the control.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "action_postprocessors", "The postprocessors which stores the control (action) values.");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "log_probability_postprocessors",
      "The postprocessors which stores the log probability of the action/control values.");
  params.addParam<std::vector<Real>>(
      "response_shift_factors",
      "A shift constant which will be used to shift the response values. This is used for the "
      "manipulation of the neural net inputs for better training efficiency.");
  params.addParam<std::vector<Real>>(
      "response_scaling_factors",
      "A normalization constant which will be used to divide the response values. This is used for "
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
      "data from the previous timesteps will be used as inputs in the training.");
  params.addParam<std::vector<unsigned int>>("num_neurons_per_layer",
                                             "The number of neurons on each hidden layer.");
  params.addParam<std::vector<std::string>>(
      "activation_function",
      "The type of activation functions to use. It is either one value "
      "or one value per hidden layer.");

  params.addParam<std::vector<Real>>(
      "action_scaling_factors",
      "Scale factor that multiplies to the NN output to obtain a physically meaningful value.");

  params.addRequiredParam<std::vector<Real>>(
      "action_standard_deviations", "Standard deviation value used while sampling the actions.");

  return params;
}

LibtorchDRLControl::LibtorchDRLControl(const InputParameters & parameters)
  : Control(parameters),
    _control_names(getParam<std::vector<std::string>>("parameters")),
    _response_names(getParam<std::vector<PostprocessorName>>("responses")),
    _action_postprocessor_names(getParam<std::vector<PostprocessorName>>("action_postprocessors")),
    _log_probability_postprocessor_names(
        getParam<std::vector<PostprocessorName>>("log_probability_postprocessors")),
    _input_timesteps(getParam<unsigned int>("input_timesteps")),
    _initialized(false),
    _action_scaling_factors(getParam<std::vector<Real>>("action_scaling_factors")),
    _action_std(getParam<std::vector<Real>>("action_standard_deviations"))
{
  // We first check if the input parameters make sense and throw errors if different parameter
  // combinations are not allowed
  conditionalParameterError("filename",
                            {"num_neurons_per_layer", "activation_function"},
                            !getParam<bool>("torch_script_format"));

  if (isParamValid("response_shift_factors"))
  {
    _response_shift_factors = getParam<std::vector<Real>>("response_shift_factors");
    if (_response_names.size() != _response_shift_factors.size())
      paramError("response_shift_factors",
                 "The number of shift factors is not the same as the number of responses!");
  }
  else
    _response_shift_factors = std::vector<Real>(_response_names.size(), 0.0);

  if (isParamValid("response_scaling_factors"))
  {
    _response_scaling_factors = getParam<std::vector<Real>>("response_scaling_factors");
    if (_response_names.size() != _response_scaling_factors.size())
      paramError(
          "response_scaling_factors",
          "The number of normalization coefficients is not the same as the number of responses!");
  }
  else
    _response_scaling_factors = std::vector<Real>(_response_names.size(), 1.0);

  if (isParamValid("action_scaling_factors"))
  {
    _action_scaling_factors = getParam<std::vector<Real>>("action_scaling_factors");
    if (_control_names.size() != _action_scaling_factors.size())
      paramError("action_scaling_factors",
                 "The number of normalization coefficients is not the same as the number of "
                 "controlled parameters!");
  }
  else
    _action_scaling_factors = std::vector<Real>(_control_names.size(), 1.0);

  if (_control_names.size() != _action_std.size())
    paramError(
        "action_standard_deviations",
        "Nunmber of action_postprocessors does not match the number of controlled parameters.");

  if (_control_names.size() != _action_postprocessor_names.size())
    paramError(
        "action_postprocessors",
        "Nunmber of action_postprocessors does not match the number of controlled parameters.");

  if (_control_names.size() != _log_probability_postprocessor_names.size())
    paramError("log_probability_postprocessors",
               "Nunmber of log_probability_postprocessors does not match the number of controlled "
               "parameters.");

#ifdef LIBTORCH_ENABLED
  _std = torch::eye(_control_names.size());
  for (unsigned int i = 0; i < _control_names.size(); ++i)
  {
    _std[i][i] = _action_std[i];
  }

  // If the user wants to read the neural net from file, we do it. We can read it from a
  // torchscript file, or we can create a shell and read back the parameters
  if (parameters.isParamSetByUser("filename"))
  {
    std::string filename = getParam<std::string>("filename");
    if (getParam<bool>("torch_script_format"))
    {
      _nn = std::make_shared<Moose::LibtorchTorchScriptNeuralNet>(filename);
    }
    else
    {
      unsigned int multiplier = getParam<bool>("use_old_responses") ? 2 : 1;
      unsigned int num_inputs = _response_names.size() * multiplier;
      unsigned int num_outputs = _control_names.size();
      std::vector<unsigned int> num_neurons_per_layer =
          getParam<std::vector<unsigned int>>("num_neurons_per_layer");
      std::vector<std::string> activation_functions =
          parameters.isParamSetByUser("activation_function")
              ? getParam<std::vector<std::string>>("activation_function")
              : std::vector<std::string>({"relu"});
      getParam<std::vector<unsigned int>>("num_neurons_per_layer");
      auto nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
          filename, num_inputs, num_outputs, num_neurons_per_layer, activation_functions);

      try
      {
        torch::load(nn, filename);
        _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(*nn);
        _console << "Loaded requested .pt file." << std::endl;
      }
      catch (...)
      {
        mooseError("The requested pytorch parameter file could not be loaded. Make sure the "
                   "dimensions of the generated neural net are the same as the dimensions of the "
                   "parameters in the input file!");
      }
    }
  }
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

    _current_response.clear();
    for (unsigned int resp_i = 0; resp_i < n_responses; ++resp_i)
      _current_response.push_back(
          (getPostprocessorValueByName(_response_names[resp_i]) - _response_shift_factors[resp_i]) *
          _response_scaling_factors[resp_i]);

    if (!_initialized)
    {
      _old_responses.clear();
      for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
        _old_responses.push_back(_current_response);
      _initialized = true;
    }

    std::vector<Real> raw_input(_current_response);
    for (unsigned int step_i = 0; step_i < num_old_timesteps; ++step_i)
      raw_input.insert(
          raw_input.end(), _old_responses[step_i].begin(), _old_responses[step_i].end());

    auto options = torch::TensorOptions().dtype(at::kDouble);
    torch::Tensor input_tensor =
        torch::from_blob(raw_input.data(), {1, _input_timesteps * n_responses}, options)
            .to(at::kDouble);

    torch::Tensor output_tensor = _nn->forward(input_tensor);

    // sample control value (action) from Gaussian distribution
    torch::Tensor action = at::normal(output_tensor, _std);

    // compute log probability
    torch::Tensor log_probability = computeLogProbability(action, output_tensor);

    // convert data
    std::vector<Real> converted_action = {action.data_ptr<Real>(),
                                          action.data_ptr<Real>() + action.size(1)};

    std::vector<Real> converted_log_probability = {log_probability.data_ptr<Real>(),
                                                   log_probability.data_ptr<Real>() +
                                                       log_probability.size(1)};

    for (unsigned int control_i = 0; control_i < n_controls; ++control_i)
    {
      // we scale the controllable value for physically meaningful control action
      setControllableValueByName<Real>(_control_names[control_i],
                                       converted_action[control_i] *
                                           _action_scaling_factors[control_i]);
      // save action values to postprocessor
      // we do not scale the action value here. it will be used and reported directly for training
      _fe_problem.setPostprocessorValueByName(_action_postprocessor_names[control_i],
                                              converted_action[control_i]);

      // save log probability values to postprocessor
      _fe_problem.setPostprocessorValueByName(_log_probability_postprocessor_names[control_i],
                                              converted_log_probability[control_i]);
    }

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
void
LibtorchDRLControl::loadControlNeuralNet(
    const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & input_nn)
{
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(*input_nn);
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
#endif

void
LibtorchDRLControl::conditionalParameterError(const std::string & param_name,
                                              const std::vector<std::string> & conditional_params,
                                              bool should_be_defined)
{
  if (parameters().isParamSetByUser(param_name))
    for (const auto & param : conditional_params)
      if (parameters().isParamSetByUser(param) != should_be_defined)
        paramError(param,
                   "This parameter should " + std::string(should_be_defined ? "" : "not") +
                       " be defined when " + param_name + " is defined!");
}
