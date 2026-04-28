//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "SamplerDRLControlTransfer.h"
#include "LibtorchDRLControl.h"

registerMooseObject("StochasticToolsApp", SamplerDRLControlTransfer);

InputParameters
SamplerDRLControlTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params += SurrogateModelInterface::validParams();

  params.addClassDescription(
      "Copies a DRL actor from a trainer object on the main app to a LibtorchDRLControl object "
      "on the subapp.");

  params.suppressParameter<MultiAppName>("from_multi_app");

  params.addRequiredParam<UserObjectName>(
      "trainer_name", "Trainer object that owns the latest controller network.");
  params.addRequiredParam<std::string>("control_name", "Controller object name.");
  return params;
}

SamplerDRLControlTransfer::SamplerDRLControlTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    SurrogateModelInterface(this),
    _control_name(getParam<std::string>("control_name")),
    _trainer(getSurrogateTrainerByName<LibtorchDRLControlTrainer>(
        getParam<UserObjectName>("trainer_name")))
{
}

void
SamplerDRLControlTransfer::initialSetup()
{
}

void
SamplerDRLControlTransfer::execute()
{
  const auto n = getToMultiApp()->numGlobalApps();
  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (getToMultiApp()->hasLocalApp(i))
    {
      // Get the control neural net from the trainer
      const Moose::LibtorchActorNeuralNet & trainer_nn = _trainer.controlNeuralNet();

      // Get the control object from the other app
      FEProblemBase & app_problem = _multi_app->appProblemBase(i);
      auto & control_warehouse = app_problem.getControlWarehouse();
      std::shared_ptr<Control> control_ptr = control_warehouse.getActiveObject(_control_name);
      LibtorchDRLControl * control_object = dynamic_cast<LibtorchDRLControl *>(control_ptr.get());

      if (!control_object)
        paramError("control_name", "The given control is not a LibtorchDRLControl!");

      // Copy and the neural net and execute it to get the initial values
      control_object->loadControlNeuralNet(trainer_nn);
      control_object->execute();
    }
  }
}

void
SamplerDRLControlTransfer::initializeFromMultiapp()
{
}

void
SamplerDRLControlTransfer::executeFromMultiapp()
{
}

void
SamplerDRLControlTransfer::finalizeFromMultiapp()
{
}

void
SamplerDRLControlTransfer::initializeToMultiapp()
{
}

void
SamplerDRLControlTransfer::executeToMultiapp()
{
  if (getToMultiApp()->hasLocalApp(_app_index))
  {
    // Use a rank-invariant seed based on the configured trainer seed, the current main-app
    // training step, and the sampler row being executed. This keeps the stochastic rollout path
    // tied to the actual sample instead of the transient local app slot chosen by batch-reset.
    const uint64_t sample_seed = static_cast<uint64_t>(_trainer.seed()) +
                                 static_cast<uint64_t>(_global_index) +
                                 static_cast<uint64_t>(_sampler_ptr->getNumberOfRows()) *
                                     static_cast<uint64_t>(_fe_problem.timeStep());
    // Get the control neural net from the trainer
    const Moose::LibtorchActorNeuralNet & trainer_nn = _trainer.controlNeuralNet();

    // Get the control object from the other app
    FEProblemBase & app_problem = _multi_app->appProblemBase(_app_index);
    auto & control_warehouse = app_problem.getControlWarehouse();
    std::shared_ptr<Control> control_ptr = control_warehouse.getActiveObject(_control_name);
    LibtorchDRLControl * control_object = dynamic_cast<LibtorchDRLControl *>(control_ptr.get());

    if (!control_object)
      paramError("control_name", "The given control is not a LibtorchDRLControl!");

    // Copy and the neural net and execute it to get the initial values
    control_object->loadControlNeuralNet(trainer_nn);
    control_object->setPolicySampleSeed(sample_seed);
    control_object->execute();
  }
}

void
SamplerDRLControlTransfer::finalizeToMultiapp()
{
}

#endif
