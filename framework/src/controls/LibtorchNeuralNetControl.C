//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibtorchNeuralNetControl.h"
#include "LibtorchTorchScriptNeuralNet.h"
#include "LibtorchArtificialNeuralNet.h"
#include "Transient.h"

registerMooseObject("MooseApp", LibtorchNeuralNetControl);

InputParameters
LibtorchNeuralNetControl::validParams()
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
      "postprocessors", "The postprocessors which stores the control values.");
  params.addParam<std::string>("filename",
                               "Define if the neural net is supposed to be loaded from a file.");
  params.addParam<bool>("torch_script_format",
                        false,
                        "If we want to load the neural net using the torch-script format.");
  params.addParam<bool>("use_old_response",
                        false,
                        "If we want to use the old responses besides the current responses to "
                        "ealuate the neural network.");
  params.addParam<std::vector<unsigned int>>("num_neurons_per_layer",
                                             "The number of neurons on each hidden layer.");
  params.addParam<std::vector<std::string>>(
      "activation_function",
      "The type of activation functions to use. It is either one value "
      "or one value per hidden layer.");

  return params;
}

LibtorchNeuralNetControl::LibtorchNeuralNetControl(const InputParameters & parameters)
  : Control(parameters),
    _executed_once(false),
    _control_names(getParam<std::vector<std::string>>("parameters")),
    _response_names(getParam<std::vector<PostprocessorName>>("responses")),
    _postprocessor_names(getParam<std::vector<PostprocessorName>>("postprocessors"))
{
  // We first check if the input parameters make sense and throw errors if different parameter
  // combinations are not allowed
  conditionalParameterError("filename",
                            {"num_neurons_per_layer", "activation_function"},
                            !getParam<bool>("torch_script_format"));

#ifdef LIBTORCH_ENABLED
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
LibtorchNeuralNetControl::execute()
{
#ifdef LIBTORCH_ENABLED
  if (_nn)
  {
    bool use_old_response = getParam<bool>("use_old_response");
    unsigned int n_responses = _response_names.size();

    _current_response.clear();
    for (unsigned int resp_i = 0; resp_i < _response_names.size(); ++resp_i)
      _current_response.push_back(getPostprocessorValueByName(_response_names[resp_i]));

    if (use_old_response && !_executed_once)
    {
      _old_response = _current_response;
      _executed_once = true;
    }

    std::vector<Real> raw_input(_current_response);
    if (use_old_response)
      raw_input.insert(raw_input.end(), _old_response.begin(), _old_response.end());

    auto options = torch::TensorOptions().dtype(at::kDouble);
    torch::Tensor input_tensor =
        torch::from_blob(raw_input.data(), {1, (use_old_response ? 2 : 1) * n_responses}, options)
            .to(at::kDouble);

    torch::Tensor output_tensor = _nn->forward(input_tensor);

    std::vector<Real> converted_output = {output_tensor.data_ptr<Real>(),
                                          output_tensor.data_ptr<Real>() + output_tensor.size(1)};

    for (unsigned int control_i = 0; control_i < _control_names.size(); ++control_i)
    {
      setControllableValueByName<Real>(_control_names[control_i], converted_output[control_i]);
      _fe_problem.setPostprocessorValueByName(_postprocessor_names[control_i],
                                              converted_output[control_i]);
    }

    _old_response = _current_response;
  }
#endif
}

#ifdef LIBTORCH_ENABLED
void
LibtorchNeuralNetControl::loadControlNeuralNet(
    const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & input_nn)
{
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(*input_nn);
}
#endif

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
                   "This parameter should " + std::string(should_be_defined ? "" : "not") +
                       " be defined when " + param_name + " is defined!");
}
