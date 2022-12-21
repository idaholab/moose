//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "DRLRewardReporter.h"

registerMooseObject("StochasticToolsApp", DRLRewardReporter);

InputParameters
DRLRewardReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += SurrogateModelInterface::validParams();

  params.addClassDescription("Reporter containing the reward values of a DRL controller trainer.");
  params.addRequiredParam<UserObjectName>(
      "drl_trainer_name", "The name of the RDL controller trainer which computes the rewards.");

  return params;
}

DRLRewardReporter::DRLRewardReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    SurrogateModelInterface(this),
    _reward(declareValueByName<Real>("average_reward", REPORTER_MODE_ROOT)),
    _trainer(getSurrogateTrainer<LibtorchDRLControlTrainer>("drl_trainer_name"))
{
}

void
DRLRewardReporter::execute()
{
  _reward = _trainer.averageEpisodeReward();
}

#endif
