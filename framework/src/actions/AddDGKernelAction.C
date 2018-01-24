//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDGKernelAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddDGKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddDGKernelAction::AddDGKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
AddDGKernelAction::act()
{
  _problem->addDGKernel(_type, _name, _moose_object_pars);
}
