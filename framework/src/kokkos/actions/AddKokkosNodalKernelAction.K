//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddKokkosNodalKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddKokkosNodalKernelAction, "add_nodal_kernel");

InputParameters
AddKokkosNodalKernelAction::validParams()
{
  InputParameters params = KokkosObjectAction::validParams();
  params.addClassDescription("Add a Kokkos NodalKernel object to the simulation.");
  return params;
}

AddKokkosNodalKernelAction::AddKokkosNodalKernelAction(const InputParameters & params)
  : KokkosObjectAction(params, "NodalKernel")
{
}

void
AddKokkosNodalKernelAction::act()
{
  _problem->addKokkosNodalKernel(_type, _name, _moose_object_pars);
}
