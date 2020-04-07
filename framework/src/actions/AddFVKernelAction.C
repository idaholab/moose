//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVKernelAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

registerMooseAction("MooseApp", AddFVKernelAction, "add_fv_kernel");

defineLegacyParams(AddFVKernelAction);

InputParameters
AddFVKernelAction::validParams()
{
  return MooseObjectAction::validParams();
}

AddFVKernelAction::AddFVKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
AddFVKernelAction::act()
{
  if (_current_task == "add_fv_kernel")
  {
    if (Registry::isADObj(_type + "<RESIDUAL>"))
    {
      _problem->addFVKernel(_type + "<RESIDUAL>", _name + "_residual", _moose_object_pars);
      _problem->addFVKernel(_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
      _problem->haveADObjects(true);
    }
    else
      _problem->addFVKernel(_type, _name, _moose_object_pars);
  }
}
