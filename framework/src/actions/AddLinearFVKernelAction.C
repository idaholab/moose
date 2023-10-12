//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLinearFVKernelAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

registerMooseAction("MooseApp", AddLinearFVKernelAction, "add_linear_fv_kernel");

InputParameters
AddLinearFVKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a LinearFVKernel object to the simulation.");
  return params;
}

AddLinearFVKernelAction::AddLinearFVKernelAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddLinearFVKernelAction::act()
{
  if (_current_task == "add_linear_fv_kernel")
    _problem->addLinearFVKernel(_type, _name, _moose_object_pars);
}
