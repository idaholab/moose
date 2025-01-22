//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "SamplerNeuralNetControlTransfer.h"
#include "LibtorchNeuralNetControl.h"

registerMooseObject("StochasticToolsApp", SamplerNeuralNetControlTransfer);

InputParameters
SamplerNeuralNetControlTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params += SurrogateModelInterface::validParams();

  params.addClassDescription("Copies a neural network from a trainer object on the main app to a "
                             "LibtorchNeuralNetControl object on the subapp.");

  params.suppressParameter<MultiAppName>("from_multi_app");

  params.addRequiredParam<UserObjectName>("trainer_name",
                                          "Trainer object that contains the neural networks."
                                          " for different samples.");
  params.addRequiredParam<std::string>("control_name", "Controller object name.");
  return params;
}

SamplerNeuralNetControlTransfer::SamplerNeuralNetControlTransfer(
    const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    SurrogateModelInterface(this),
    _control_name(getParam<std::string>("control_name")),
    _trainer(getSurrogateTrainerByName<LibtorchDRLControlTrainer>(
        getParam<UserObjectName>("trainer_name")))
{
}

void
SamplerNeuralNetControlTransfer::initialSetup()
{
  const auto multi_app = getToMultiApp();
  const dof_id_type n = multi_app->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
    if (multi_app->hasLocalApp(i))
      torch::manual_seed(i);
}

void
SamplerNeuralNetControlTransfer::execute()
{
  const auto n = getToMultiApp()->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    // std::cout << "Do I have this app? " << i << " " << getToMultiApp()->hasLocalApp(i) << std::endl;
    if (getToMultiApp()->hasLocalApp(i))
    {
      // Get the control neural net from the trainer
      const Moose::LibtorchArtificialNeuralNet & trainer_nn = _trainer.controlNeuralNet();

      // Get the control object from the other app
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);
      auto & control_warehouse = app_problem.getControlWarehouse();
      std::shared_ptr<Control> control_ptr = control_warehouse.getActiveObject(_control_name);
      LibtorchNeuralNetControl * control_object =
      dynamic_cast<LibtorchNeuralNetControl *>(control_ptr.get());

      if (!control_object)
        paramError("control_name", "The given control is not a LibtorchNeuralNetrControl!");

      // Copy and the neural net and execute it to get the initial values
      control_object->loadControlNeuralNet(trainer_nn);
      control_object->execute();

      // const auto & named_params = trainer_nn.named_parameters();
      // for (const auto & param_i : make_range(named_params.size()))
      // {
      //   // We cast the parameters into a 1D vector
      //   std::cout << "Transferring " << Moose::stringify(std::vector<Real>(
      //       named_params[param_i].value().data_ptr<Real>(),
      //       named_params[param_i].value().data_ptr<Real>() + named_params[param_i].value().numel())) << std::endl;
      // }
    }
  }
}

void
SamplerNeuralNetControlTransfer::initializeFromMultiapp()
{
}

void
SamplerNeuralNetControlTransfer::executeFromMultiapp()
{
}

void
SamplerNeuralNetControlTransfer::finalizeFromMultiapp()
{
}

void
SamplerNeuralNetControlTransfer::initializeToMultiapp()
{
}

void
SamplerNeuralNetControlTransfer::executeToMultiapp()
{
  if (getToMultiApp()->hasLocalApp(_global_index))
  {
    // Get the control neural net from the trainer
    const Moose::LibtorchArtificialNeuralNet & trainer_nn = _trainer.controlNeuralNet();

    // Get the control object from the other app
    FEProblemBase & app_problem = _multi_app->appProblemBase(_global_index);
    auto & control_warehouse = app_problem.getControlWarehouse();
    std::shared_ptr<Control> control_ptr = control_warehouse.getActiveObject(_control_name);
    LibtorchNeuralNetControl * control_object =
    dynamic_cast<LibtorchNeuralNetControl *>(control_ptr.get());

    if (!control_object)
      paramError("control_name", "The given control is not a LibtorchNeuralNetrControl!");

    // Copy and the neural net and execute it to get the initial values
    control_object->loadControlNeuralNet(trainer_nn);
    control_object->execute();
  }
}

void
SamplerNeuralNetControlTransfer::finalizeToMultiapp()
{
}

#endif
