//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDELEMENTALFIELDACTION_H
#define ADDELEMENTALFIELDACTION_H

#include "Action.h"

class AddElementalFieldAction;

template <>
InputParameters validParams<AddElementalFieldAction>();

class AddElementalFieldAction : public Action
{
public:
  AddElementalFieldAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDELEMENTALFIELDACTION_H
