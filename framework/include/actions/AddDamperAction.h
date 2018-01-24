//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDDAMPERACTION_H
#define ADDDAMPERACTION_H

#include "MooseObjectAction.h"

class AddDamperAction;

template <>
InputParameters validParams<AddDamperAction>();

class AddDamperAction : public MooseObjectAction
{
public:
  AddDamperAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDDAMPERACTION_H
