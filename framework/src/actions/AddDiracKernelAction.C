//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDiracKernelAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddDiracKernelAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddDiracKernelAction::AddDiracKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
AddDiracKernelAction::act()
{
  _problem->addDiracKernel(_type, _name, _moose_object_pars);
}
