//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "AddMFEMFESpaceAction.h"

registerMooseAction("MooseApp", AddMFEMFESpaceAction, "add_mfem_fespaces");

InputParameters
AddMFEMFESpaceAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM FESpace object to the simulation.");
  return params;
}

AddMFEMFESpaceAction::AddMFEMFESpaceAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMFESpaceAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addFESpace(_type, _name, _moose_object_pars);
}

#endif
