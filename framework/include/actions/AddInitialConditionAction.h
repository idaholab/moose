//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDINITIALCONDITIONACTION_H
#define ADDINITIALCONDITIONACTION_H

#include "MooseObjectAction.h"

class AddInitialConditionAction;

template <>
InputParameters validParams<AddInitialConditionAction>();

class AddInitialConditionAction : public MooseObjectAction
{
public:
  AddInitialConditionAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDINITIALCONDITIONACTION_H
