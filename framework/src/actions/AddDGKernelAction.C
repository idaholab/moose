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
#include "NonlinearSystem.h"

registerMooseAction("MooseApp", AddDGKernelAction, "add_dg_kernel");
registerMooseAction("MooseApp", AddDGKernelAction, "ready_to_init");

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
  if (_current_task == "ready_to_init")
    _problem->getNonlinearSystem().dofMap().set_implicit_neighbor_dofs(true);

  if (_current_task == "add_dg_kernel")
    _problem->addDGKernel(_type, _name, _moose_object_pars);
}
