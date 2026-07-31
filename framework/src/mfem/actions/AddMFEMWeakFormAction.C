//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMWeakFormAction.h"

registerMooseAction("MooseApp", AddMFEMWeakFormAction, "add_mfem_weak_form");

InputParameters
AddMFEMWeakFormAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM WeakForm object to the simulation.");
  return params;
}

AddMFEMWeakFormAction::AddMFEMWeakFormAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMWeakFormAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).addWeakForm(_type, _name, _moose_object_pars);
}

#endif
