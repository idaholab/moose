//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMPeriodicBCs.h"

registerMooseAction("MooseApp", AddMFEMPeriodicBCs, "add_mfem_periodic_bcs");

InputParameters
AddMFEMPeriodicBCs::validParams()
{
  InputParameters params = Action::validParams();
  // params.addClassDescription("Add a MFEM FESpace object to the simulation.");
  return params;
}

AddMFEMPeriodicBCs::AddMFEMPeriodicBCs(const InputParameters & parameters)
  : Action(parameters)
{
}

void
AddMFEMPeriodicBCs::act()
{
  std::cout << "We are acting!!\n";
}

#endif
