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

registerMooseAction("MooseApp", AddMFEMProblemOperatorAction, "add_mfem_problem_operator");

InputParameters
AddMFEMProblemOperatorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a problem operator to the MFEM problem.");
  return params;
}

AddMFEMProblemOperatorAction::AddMFEMProblemOperatorAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMProblemOperatorAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).addMFEMProblemOperator(_type, _name, _moose_object_pars);
}

#endif
