//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "AddMFEMSolverAction.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp", AddMFEMSolverAction, "add_mfem_solver");

InputParameters
AddMFEMSolverAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Set the Moose::MFEM solver and the solver options.");
  return params;
}

AddMFEMSolverAction::AddMFEMSolverAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMSolverAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addMFEMSolver(_type, _name, _moose_object_pars);
}

#endif
