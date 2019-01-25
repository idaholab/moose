//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddADDGKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddADDGKernelAction, "add_ad_dg_kernel");

template <>
InputParameters
validParams<AddADDGKernelAction>()
{
  return validParams<MooseADObjectAction>();
}

AddADDGKernelAction::AddADDGKernelAction(InputParameters params) : MooseADObjectAction(params) {}

void
AddADDGKernelAction::act()
{
  _problem->addDGKernel(_base_type + "<RESIDUAL>", _name + "_residual", _moose_object_pars);
  _problem->addDGKernel(_base_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
}
