//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPTIMESTEPPERACTION_H
#define SETUPTIMESTEPPERACTION_H

#include "MooseObjectAction.h"

class SetupTimeStepperAction;

template <>
InputParameters validParams<SetupTimeStepperAction>();

/**
 *
 */
class SetupTimeStepperAction : public MooseObjectAction
{
public:
  SetupTimeStepperAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* SETUPTIMESTEPPERACTION_H */
