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

registerMooseAction("MooseApp", AddKokkosKernelAction, "add_kernel");

InputParameters
AddKokkosKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Kokkos Kernel object to the simulation.");
  return params;
}

AddKokkosKernelAction::AddKokkosKernelAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddKokkosKernelAction::act()
{
#ifndef MOOSE_HAVE_KOKKOS
  mooseError("Attempted to add a Kokkos kernel but MOOSE was not compiled with Kokkos support.");
#else
  if (!_app.hasGPUs())
    mooseError("Attempted to add a Kokkos kernel but no GPU was detected in the system.");
  else
    _problem->addKokkosKernel(_type, _name, _moose_object_pars);
#endif
}
