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

#include <numeric>

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

      LibtorchDRLControl & control_object = getDRLControl(i);

      // Copy and the neural net and execute it to get the initial values
      control_object.loadControlNeuralNet(trainer_nn);
      control_object.execute();
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

    LibtorchDRLControl & control_object = getDRLControl(_app_index);

    // Copy and the neural net and execute it to get the initial values
    control_object.loadControlNeuralNet(trainer_nn);
    control_object.setPolicySampleSeed(sample_seed);
    control_object.execute();
  }
}

LibtorchDRLControl &
SamplerDRLControlTransfer::getDRLControl(const unsigned int app_index)
{
  // Get the control object from the other app
  FEProblemBase & app_problem = _multi_app->appProblemBase(app_index);
  auto & control_warehouse = app_problem.getControlWarehouse();
  std::shared_ptr<Control> control_ptr = control_warehouse.getActiveObject(_control_name);
  LibtorchDRLControl * control_object = dynamic_cast<LibtorchDRLControl *>(control_ptr.get());

  if (!control_object)
    paramError("control_name", "The given control is not a LibtorchDRLControl!");

  // The trainer builds one transition and one history lag per timestep_window steps, so any
  // other window mixes held actions or skips policy evaluations made by the control.
  const unsigned int timestep_window = _trainer.timestepWindow();
  const unsigned int num_steps_in_period = control_object->numStepsInPeriod();
  if (timestep_window != num_steps_in_period)
  {
    const auto divisor = std::gcd(timestep_window, num_steps_in_period);
    const auto numerator = std::to_string(timestep_window / divisor);
    const auto denominator = num_steps_in_period / divisor;
    const std::string ratio = denominator == 1
                                  ? numerator + " times"
                                  : numerator + "/" + std::to_string(denominator) + " of";
    paramError("control_name",
               "The trainer '",
               _trainer.name(),
               "' has timestep_window = ",
               timestep_window,
               ", which is ",
               ratio,
               " this control's num_steps_in_period = ",
               num_steps_in_period,
               ". They must be equal so that each training transition corresponds to exactly one "
               "policy evaluation.");
  }

  if (_trainer.inputTimesteps() != control_object->inputTimesteps())
    paramError("control_name",
               "The trainer '",
               _trainer.name(),
               "' has input_timesteps = ",
               _trainer.inputTimesteps(),
               ", but this control has input_timesteps = ",
               control_object->inputTimesteps(),
               ". They must be equal so that the control stacks the same observation history "
               "that the actor was trained on.");

  if (_trainer.numberOfObservations() != control_object->numberOfObservations())
    paramError("control_name",
               "The trainer '",
               _trainer.name(),
               "' reads ",
               _trainer.numberOfObservations(),
               " observation(s), but this control reads ",
               control_object->numberOfObservations(),
               ". They must be equal so that the actor receives the observations it was trained "
               "on.");

  if (_trainer.numberOfControlSignals() != control_object->numberOfControlSignals())
    paramError("control_name",
               "The trainer '",
               _trainer.name(),
               "' produces ",
               _trainer.numberOfControlSignals(),
               " control signal(s), but this control sets ",
               control_object->numberOfControlSignals(),
               " parameter(s). They must be equal so that each actor output drives exactly one "
               "controllable parameter.");

  return *control_object;
}

void
SamplerDRLControlTransfer::finalizeToMultiapp()
{
}

#endif
