//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDMULTIAPPACTION_H
#define ADDMULTIAPPACTION_H

#include "MooseObjectAction.h"

class AddMultiAppAction;

template <>
InputParameters validParams<AddMultiAppAction>();

class AddMultiAppAction : public MooseObjectAction
{
public:
  AddMultiAppAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDMULTIAPPACTION_H
