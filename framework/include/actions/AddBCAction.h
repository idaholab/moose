//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDBCACTION_H
#define ADDBCACTION_H

#include "MooseObjectAction.h"

class AddBCAction;

template <>
InputParameters validParams<AddBCAction>();

class AddBCAction : public MooseObjectAction
{
public:
  AddBCAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDBCACTION_H
