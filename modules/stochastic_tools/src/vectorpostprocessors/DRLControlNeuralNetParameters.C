//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DRLControlNeuralNetParameters.h"
#include "LibtorchDRLControlTrainer.h"

registerMooseObject("StochasticToolsApp", DRLControlNeuralNetParameters);

InputParameters
DRLControlNeuralNetParameters::validParams()
{
  InputParameters params = LibtorchArtificialNeuralNetParameters::validParams();

  params.addClassDescription("Outputs the parameters of a LibtorchArtificialNeuralNetwork within a "
                             "LibtorchDRLControlTrainer.");

  params.set<std::string>("control_name") = "";
  params.suppressParameter<std::string>("control_name");

  params.addRequiredParam<UserObjectName>("trainer_name",
                                          "The control object holding the neural network.");

  return params;
}

DRLControlNeuralNetParameters::DRLControlNeuralNetParameters(const InputParameters & params)
  : LibtorchArtificialNeuralNetParameters(params),
    SurrogateModelInterface(this),
    _trainer_name(getParam<UserObjectName>("trainer_name"))
{
}

void
DRLControlNeuralNetParameters::execute()
{
#ifdef LIBTORCH_ENABLED

  _nn_parameter_values.clear();

  const auto & trainer = getSurrogateTrainerByName<LibtorchDRLControlTrainer>(_trainer_name);

  const std::shared_ptr<torch::nn::Module> ann =
      std::dynamic_pointer_cast<torch::nn::Module>(trainer.controlNeuralNet());

  if (ann)
    fillParameterValues(ann);
  else
    mooseError("The supplied neural network from the controller could not be cast into a "
               "troch::nn::Module!");

#endif
}
