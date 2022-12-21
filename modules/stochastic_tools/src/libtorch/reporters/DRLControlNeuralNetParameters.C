//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "DRLControlNeuralNetParameters.h"
#include "LibtorchDRLControlTrainer.h"

registerMooseObject("StochasticToolsApp", DRLControlNeuralNetParameters);

InputParameters
DRLControlNeuralNetParameters::validParams()
{
  InputParameters params = GeneralReporter::validParams();

  params.addClassDescription("Outputs the parameters of a LibtorchArtificialNeuralNetwork within a "
                             "LibtorchDRLControlTrainer.");

  params.addRequiredParam<UserObjectName>("trainer_name",
                                          "The control object holding the neural network.");

  return params;
}

DRLControlNeuralNetParameters::DRLControlNeuralNetParameters(const InputParameters & params)
  : GeneralReporter(params), SurrogateModelInterface(this)
{
  auto & network =
      declareValueByName<const Moose::LibtorchArtificialNeuralNet *>(name(), REPORTER_MODE_ROOT);
  network = &getSurrogateTrainer<LibtorchDRLControlTrainer>("trainer_name").controlNeuralNet();
}

#endif
