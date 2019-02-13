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

registerMooseAction("MooseApp", AddDGKernelAction, "add_dg_kernel");

template <>
InputParameters
validParams<AddDGKernelAction>()
{
  return validParams<MooseADObjectAction>();
}

AddDGKernelAction::AddDGKernelAction(InputParameters params) : MooseADObjectAction(params) {}

void
AddDGKernelAction::act()
{
  if (Registry::isADObj(_type))
  {
    _problem->addDGKernel(_base_type + "<RESIDUAL>", _name + "_residual", _moose_object_pars);
    _problem->addDGKernel(_base_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
  }
  else
    _problem->addDGKernel(_type, _name, _moose_object_pars);
}
