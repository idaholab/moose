//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddHDGKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddHDGKernelAction, "add_hybridized_kernel");

InputParameters
AddHDGKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a hybridized kernel object to the simulation.");
  return params;
}

AddHDGKernelAction::AddHDGKernelAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddHDGKernelAction::act()
{
  _problem->addHDGKernel(_type, _name, _moose_object_pars);
}
