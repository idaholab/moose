//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseRandom.h"

class IntermittentFailureUO : public GeneralUserObject
{
public:
  static InputParameters validParams();

  IntermittentFailureUO(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

  virtual void initialize() override {}
  virtual void finalize() override {}

private:
  enum class FailureType
  {
    RUN_SLOW
  };

  const FileName & _state_file;

  const unsigned int _timestep_to_fail;

  const FailureType _failure_type;

  bool _will_fail_this_run;
};
