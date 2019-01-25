//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDADDGKERNELACTION_H
#define ADDADDGKERNELACTION_H

#include "MooseADObjectAction.h"

class AddADDGKernelAction;

template <>
InputParameters validParams<AddADDGKernelAction>();

class AddADDGKernelAction : public MooseADObjectAction
{
public:
  AddADDGKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDADDGKERNELACTION_H
