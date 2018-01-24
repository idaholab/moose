//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDINTERFACEKERNELACTION_H
#define ADDINTERFACEKERNELACTION_H

#include "MooseObjectAction.h"

class AddInterfaceKernelAction;

template <>
InputParameters validParams<AddInterfaceKernelAction>();

class AddInterfaceKernelAction : public MooseObjectAction
{
public:
  AddInterfaceKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDKERNELACTION_H
