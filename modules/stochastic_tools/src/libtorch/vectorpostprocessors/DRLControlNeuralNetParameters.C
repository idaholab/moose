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
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addClassDescription("Outputs the parameters of a LibtorchArtificialNeuralNetwork within a "
                             "LibtorchDRLControlTrainer.");

  params.set<std::string>("control_name") = "";
  params.suppressParameter<std::string>("control_name");

  params.addRequiredParam<UserObjectName>("trainer_name",
                                          "The control object holding the neural network.");

  return params;
}

DRLControlNeuralNetParameters::DRLControlNeuralNetParameters(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    SurrogateModelInterface(this),
    _nn_parameter_values(declareVector("nn_parameter_values")),
    _ann(getSurrogateTrainer<LibtorchDRLControlTrainer>("trainer_name").controlNeuralNet())
{
  if (!_ann)
    mooseError("Failed to fetch the neural network from the DRLControlTrainer!");
}

void
DRLControlNeuralNetParameters::execute()
{
#ifdef LIBTORCH_ENABLED

  _nn_parameter_values.clear();

  if (_ann)
    LibtorchArtificialNeuralNetParameters::fillParameterValues(_nn_parameter_values, _ann);
  else
    mooseError("The supplied neural network from the controller could not be cast into a "
               "troch::nn::Module!");

#endif
}
