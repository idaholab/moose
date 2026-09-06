//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMProblemComposerAction.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp", AddMFEMProblemComposerAction, "add_mfem_problem_composer");

InputParameters
AddMFEMProblemComposerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Problem Composer to the MFEM problem.");
  return params;
}

AddMFEMProblemComposerAction::AddMFEMProblemComposerAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMProblemComposerAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).addMFEMProblemComposer(_type, _name, _moose_object_pars);
}

#endif
