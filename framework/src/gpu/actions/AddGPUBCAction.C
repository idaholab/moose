//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGPUBCAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddGPUBCAction, "add_bc");

InputParameters
AddGPUBCAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a GPU Kernel object to the simulation.");
  return params;
}

AddGPUBCAction::AddGPUBCAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddGPUBCAction::act()
{
#ifndef MOOSE_HAVE_GPU
  mooseError(
      "Attempted to add a GPU boundary condition but MOOSE was not compiled with GPU support.");
#else
  if (!_app.hasGPUs())
    mooseError("Attempted to add a GPU boundary condition but no GPU was detected in the system.");
  else
    _problem->addGPUBoundaryCondition(_type, _name, _moose_object_pars);
#endif
}
