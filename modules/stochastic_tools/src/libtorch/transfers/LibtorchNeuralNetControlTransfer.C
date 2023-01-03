//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchNeuralNetControlTransfer.h"
#include "LibtorchNeuralNetControl.h"

registerMooseObject("StochasticToolsApp", LibtorchNeuralNetControlTransfer);

InputParameters
LibtorchNeuralNetControlTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params += SurrogateModelInterface::validParams();

  params.addClassDescription("Copies a neural network from a trainer object on the main app to a "
                             "LibtorchNeuralNetControl object on the subapp.");

  params.suppressParameter<MultiAppName>("from_multi_app");
  params.suppressParameter<MultiAppName>("multi_app");
  params.suppressParameter<MultiMooseEnum>("direction");

  params.addRequiredParam<UserObjectName>("trainer_name",
                                          "Trainer object that contains the neural networks."
                                          " for different samples.");
  params.addRequiredParam<std::string>("control_name", "Controller object name.");
  return params;
}

LibtorchNeuralNetControlTransfer::LibtorchNeuralNetControlTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    SurrogateModelInterface(this),
    _control_name(getParam<std::string>("control_name")),
    _trainer(getSurrogateTrainerByName<LibtorchDRLControlTrainer>(
        getParam<UserObjectName>("trainer_name")))
{
}

void
LibtorchNeuralNetControlTransfer::execute()
{
  // Get the control neural net from the trainer
  const Moose::LibtorchArtificialNeuralNet & trainer_nn = _trainer.controlNeuralNet();

  // Get the control object from the other app
  FEProblemBase & app_problem = _multi_app->appProblemBase(0);
  auto & control_warehouse = app_problem.getControlWarehouse();
  std::shared_ptr<Control> control_ptr = control_warehouse.getActiveObject(_control_name);
  LibtorchNeuralNetControl * control_object =
      dynamic_cast<LibtorchNeuralNetControl *>(control_ptr.get());

  if (!control_object)
    paramError("control_name", "The given gontrol is not a LibtorchNeuralNetrControl!");

  // Copy and the neural net and execute it to get the initial values
  control_object->loadControlNeuralNet(trainer_nn);
  control_object->execute();
}
#endif
