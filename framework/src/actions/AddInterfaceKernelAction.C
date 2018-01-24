//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddInterfaceKernelAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddInterfaceKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddInterfaceKernelAction::AddInterfaceKernelAction(InputParameters params)
  : MooseObjectAction(params)
{
}

void
AddInterfaceKernelAction::act()
{
  _problem->addInterfaceKernel(_type, _name, _moose_object_pars);
}
