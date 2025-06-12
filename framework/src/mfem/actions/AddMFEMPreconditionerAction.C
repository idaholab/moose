//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "AddMFEMPreconditionerAction.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp", AddMFEMPreconditionerAction, "add_mfem_preconditioner");

InputParameters
AddMFEMPreconditionerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a preconditioner to the MFEM problem.");
  return params;
}

AddMFEMPreconditionerAction::AddMFEMPreconditionerAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMPreconditionerAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addMFEMPreconditioner(_type, _name, _moose_object_pars);
}

#endif
