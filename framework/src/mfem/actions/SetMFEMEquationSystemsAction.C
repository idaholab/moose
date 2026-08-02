//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "SetMFEMEquationSystemsAction.h"

registerMooseAction("MooseApp", SetMFEMEquationSystemsAction, "add_mfem_equation_systems");

InputParameters
SetMFEMEquationSystemsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set EquationSystem operators to solve in this problem.");
  return params;
}

SetMFEMEquationSystemsAction::SetMFEMEquationSystemsAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
SetMFEMEquationSystemsAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).setEquationSystems();
}

#endif
