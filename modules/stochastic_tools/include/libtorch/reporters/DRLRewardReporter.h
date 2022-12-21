//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include "LibtorchDRLControlTrainer.h"
#include "GeneralReporter.h"
#include "SurrogateModelInterface.h"

/// Reporter which saves the reward values from a Deep Reinforcement Learning controller trainer
class DRLRewardReporter : public GeneralReporter, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  DRLRewardReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /// The reward values which will be saved
  Real & _reward;
  /// The DRL trainer which computes the reward values
  LibtorchDRLControlTrainer & _trainer;
};

#endif
