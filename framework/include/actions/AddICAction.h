//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDICACTION_H
#define ADDICACTION_H

#include "MooseObjectAction.h"

class AddICAction;

template <>
InputParameters validParams<AddICAction>();

class AddICAction : public MooseObjectAction
{
public:
  AddICAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDICACTION_H
