//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDSCALARKERNELACTION_H
#define ADDSCALARKERNELACTION_H

#include "MooseObjectAction.h"

class AddScalarKernelAction;

template <>
InputParameters validParams<AddScalarKernelAction>();

class AddScalarKernelAction : public MooseObjectAction
{
public:
  AddScalarKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDSCALARKERNELACTION_H
