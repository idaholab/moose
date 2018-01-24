//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddScalarKernelAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddScalarKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddScalarKernelAction::AddScalarKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
AddScalarKernelAction::act()
{
  if (_current_task == "add_scalar_kernel")
    _problem->addScalarKernel(_type, _name, _moose_object_pars);
  else
    _problem->addAuxScalarKernel(_type, _name, _moose_object_pars);
}
