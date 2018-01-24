//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDDIRACKERNELACTION_H
#define ADDDIRACKERNELACTION_H

#include "MooseObjectAction.h"

class AddDiracKernelAction;

template <>
InputParameters validParams<AddDiracKernelAction>();

class AddDiracKernelAction : public MooseObjectAction
{
public:
  AddDiracKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDDIRACKERNELACTION_H
