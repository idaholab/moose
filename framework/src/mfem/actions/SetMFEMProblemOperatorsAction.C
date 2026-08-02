//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "SetMFEMProblemOperatorsAction.h"

registerMooseAction("MooseApp", SetMFEMProblemOperatorsAction, "set_mfem_problem_operators");

InputParameters
SetMFEMProblemOperatorsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set ProblemOperators to solve in this problem.");
  return params;
}

SetMFEMProblemOperatorsAction::SetMFEMProblemOperatorsAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
SetMFEMProblemOperatorsAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).setMFEMProblemOperators();
}

#endif
