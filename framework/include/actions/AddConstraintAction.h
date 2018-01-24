//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDCONSTRAINTACTION_H
#define ADDCONSTRAINTACTION_H

#include "MooseObjectAction.h"

class AddConstraintAction;

template <>
InputParameters validParams<AddConstraintAction>();

class AddConstraintAction : public MooseObjectAction
{
public:
  AddConstraintAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDCONSTRAINTACTION_H
