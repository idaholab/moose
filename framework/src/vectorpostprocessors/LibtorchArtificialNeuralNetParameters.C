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
#endif

#include "LibtorchArtificialNeuralNetParameters.h"

registerMooseObject("MooseApp", LibtorchArtificialNeuralNetParameters);

InputParameters
LibtorchArtificialNeuralNetParameters::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addClassDescription("Outputs the parameters of a LibtorchArtificialNeuralNetwork within a "
                             "LibtorchNeuralNetControl.");

  params.addRequiredParam<std::string>("control_name",
                                       "The control object holding the neural network.");

  return params;
}

LibtorchArtificialNeuralNetParameters::LibtorchArtificialNeuralNetParameters(
    const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _control_name(getParam<std::string>("control_name")),
    _nn_parameter_values(declareVector("nn_parameter_values"))
{
}

void
LibtorchArtificialNeuralNetParameters::execute()
{
#ifdef LIBTORCH_ENABLED

  _nn_parameter_values.clear();

  auto & control_warehouse = _fe_problem.getControlWarehouse();
  const auto & control_ref =
      control_warehouse.getActiveObject(getParam<std::string>("control_name"));
  std::shared_ptr<LibtorchNeuralNetControl> control_object =
      std::dynamic_pointer_cast<LibtorchNeuralNetControl>(control_ref);

  const std::shared_ptr<torch::nn::Module> ann =
      std::dynamic_pointer_cast<torch::nn::Module>(control_object->controlNeuralNet());

  if (ann)
    fillParameterValues(ann);
  else
    mooseError("The supplied neural network from the controller could not be cast into a "
               "troch::nn::Module!");

#endif
}

#ifdef LIBTORCH_ENABLED
void
LibtorchArtificialNeuralNetParameters::fillParameterValues(
    const std::shared_ptr<torch::nn::Module> & ann)
{

  const auto & ann_params = ann->named_parameters();

  for (unsigned int param_i : make_range(ann_params.size()))
  {
    auto sizes = ann_params[param_i].value().data().sizes();
    unsigned int max_size = 1;
    for (const auto & dim_size : sizes)
      max_size *= dim_size;

    auto flattened_tensor = ann_params[param_i].value().data().reshape({max_size, 1});
    for (unsigned int value_i : make_range(max_size))
      _nn_parameter_values.push_back(flattened_tensor[value_i][0].item<double>());
  }
}
#endif
