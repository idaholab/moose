//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ResolveMFEMSolversAction.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp", ResolveMFEMSolversAction, "resolve_mfem_solvers");

InputParameters
ResolveMFEMSolversAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Resolve and construct Moose::MFEM solver objects.");
  return params;
}

ResolveMFEMSolversAction::ResolveMFEMSolversAction(const InputParameters & params) : Action(params)
{
}

void
ResolveMFEMSolversAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).resolveMFEMSolvers();
}

#endif
