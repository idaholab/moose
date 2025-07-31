//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMProblemOperatorAction.h"
#include "MFEMProblem.h"
#include "ProblemOperatorInterface.h"

registerMooseAction("MooseApp", AddMFEMProblemOperatorAction, "add_mfem_problem_operator");

InputParameters
AddMFEMProblemOperatorAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set the ProblemOperator used in the MFEMProblemSolve to solve the FE problem.");
  return params;
}

AddMFEMProblemOperatorAction::AddMFEMProblemOperatorAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
AddMFEMProblemOperatorAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addProblemOperator();
}

#endif
