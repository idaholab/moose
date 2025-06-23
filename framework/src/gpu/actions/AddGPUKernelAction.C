//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGPUKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddGPUKernelAction, "add_kernel");

InputParameters
AddGPUKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a GPUKernel object to the simulation.");
  return params;
}

AddGPUKernelAction::AddGPUKernelAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddGPUKernelAction::act()
{
#ifndef MOOSE_HAVE_GPU
  mooseError("Attempted to add a GPU kernel but MOOSE was not compiled with GPU support.");
#else
  if (!_app.hasGPUs())
    mooseError("Attempted to add a GPU kernel but no GPU was detected in the system.");
  else
    _problem->addGPUKernel(_type, _name, _moose_object_pars);
#endif
}
