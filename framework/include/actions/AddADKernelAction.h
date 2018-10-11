//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDADKERNELACTION_H
#define ADDADKERNELACTION_H

#include "MooseADObjectAction.h"

class AddADKernelAction;

template <>
InputParameters validParams<AddADKernelAction>();

class AddADKernelAction : public MooseADObjectAction
{
public:
  AddADKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDADKERNELACTION_H
