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
  InputParameters params = KokkosObjectAction::validParams();
  params.addClassDescription("Add a Kokkos Kernel object to the simulation.");
  return params;
}

AddKokkosKernelAction::AddKokkosKernelAction(const InputParameters & params)
  : KokkosObjectAction(params, "Kernel")
{
}

void
AddKokkosKernelAction::act()
{
  _problem->addKokkosKernel(_type, _name, _moose_object_pars);
}
