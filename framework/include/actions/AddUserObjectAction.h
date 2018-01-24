//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDUSEROBJECTACTION_H
#define ADDUSEROBJECTACTION_H

#include "MooseObjectAction.h"

class AddUserObjectAction;

template <>
InputParameters validParams<AddUserObjectAction>();

class AddUserObjectAction : public MooseObjectAction
{
public:
  AddUserObjectAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDUSEROBJECTACTION_H
