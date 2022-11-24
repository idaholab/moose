//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVInterfaceKernelAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

registerMooseAction("MooseApp", AddFVInterfaceKernelAction, "add_fv_ik");

InputParameters
AddFVInterfaceKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a FVInterfaceKernel object to the simulation.");
  return params;
}

AddFVInterfaceKernelAction::AddFVInterfaceKernelAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddFVInterfaceKernelAction::act()
{
  if (_current_task == "add_fv_ik")
    _problem->addFVInterfaceKernel(_type, _name, _moose_object_pars);
}
