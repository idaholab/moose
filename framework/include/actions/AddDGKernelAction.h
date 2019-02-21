//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDDGKERNELACTION_H
#define ADDDGKERNELACTION_H

#include "MooseObjectAction.h"

class AddDGKernelAction;

template <>
InputParameters validParams<AddDGKernelAction>();

class AddDGKernelAction : public MooseObjectAction
{
public:
  AddDGKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDKERNELACTION_H
