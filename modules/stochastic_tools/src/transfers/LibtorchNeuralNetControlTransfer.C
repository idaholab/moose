//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LibtorchNeuralNetControlTransfer.h"
#include "LibtorchNeuralNetControl.h"

registerMooseObject("StochasticToolsApp", LibtorchNeuralNetControlTransfer);

InputParameters
LibtorchNeuralNetControlTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params += SurrogateModelInterface::validParams();
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
#ifdef LIBTORCH_ENABLED
  // Selecting the appropriate action based on the drection.
  switch (_direction)
  {
    case FROM_MULTIAPP:

      mooseError("This transfer does not support transfer from the subapps.");
      break;

    case TO_MULTIAPP:

      // Get the control neural net from the trainer
      const std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & trainer_nn =
          _trainer.controlNeuralNet();

      // Get the control object from the other app
      FEProblemBase & app_problem = _multi_app->appProblemBase(0);
      auto & control_warehouse = app_problem.getControlWarehouse();
      const auto & control_ref = control_warehouse.getActiveObject(_control_name);
      std::shared_ptr<LibtorchNeuralNetControl> control_object =
          std::dynamic_pointer_cast<LibtorchNeuralNetControl>(control_ref);

      if (!control_object)
        mooseError("Couldn't cast Control object to LibtorchNeuralNetrControl!");

      // Copy and the neural net and execute it to get the initial values
      control_object->loadControlNeuralNet(trainer_nn);
      control_object->execute();

      break;
  }
#endif
}
