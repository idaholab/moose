//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDINDICATORACTION_H
#define ADDINDICATORACTION_H

#include "MooseObjectAction.h"

class AddIndicatorAction;

template <>
InputParameters validParams<AddIndicatorAction>();

class AddIndicatorAction : public MooseObjectAction
{
public:
  AddIndicatorAction(InputParameters params);

  virtual void act() override;

private:
};

#endif // ADDINDICATORACTION_H
