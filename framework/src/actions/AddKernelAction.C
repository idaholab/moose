//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddKernelAction, "add_kernel");

registerMooseAction("MooseApp", AddKernelAction, "add_aux_kernel");

InputParameters
AddKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Kernel object to the simulation.");
  return params;
}

AddKernelAction::AddKernelAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddKernelAction::act()
{
  if (_current_task == "add_kernel")
    _problem->addKernel(_type, _name, _moose_object_pars);
  else
  {
    if (getAllTasks().find("add_aux_bc") != getAllTasks().end())
      mooseWarning("The [AuxBCs] block is deprecated, all AuxKernels including both block and "
                   "boundary restricted should be added within the [AuxKernels] block");

    _problem->addAuxKernel(_type, _name, _moose_object_pars);
  }
}
