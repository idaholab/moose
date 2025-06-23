//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGPUNodalKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddGPUNodalKernelAction, "add_nodal_kernel");

InputParameters
AddGPUNodalKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a GPUNodalKernel object to the simulation.");
  return params;
}

AddGPUNodalKernelAction::AddGPUNodalKernelAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddGPUNodalKernelAction::act()
{
#ifndef MOOSE_HAVE_GPU
  mooseError("Attempted to add a GPU nodal kernel but MOOSE was not compiled with GPU support.");
#else
  if (!_app.hasGPUs())
    mooseError("Attempted to add a GPU nodal kernel but no GPU was detected in the system.");
  else
    _problem->addGPUNodalKernel(_type, _name, _moose_object_pars);
#endif
}
