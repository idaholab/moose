//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LibtorchDRLControlTrainer.h"
#include "GeneralReporter.h"
#include "SurrogateModelInterface.h"

class DRLRewardReporter : public GeneralReporter, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  DRLRewardReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  Real & _reward;
  LibtorchDRLControlTrainer & _trainer;
};
