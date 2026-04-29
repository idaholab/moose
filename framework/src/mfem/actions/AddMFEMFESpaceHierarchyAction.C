//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMFESpaceHierarchyAction.h"

registerMooseAction("MooseApp", AddMFEMFESpaceHierarchyAction, "add_mfem_fespace_hierarchies");

InputParameters
AddMFEMFESpaceHierarchyAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Moose::MFEM::FESpaceHierarchy object to the simulation.");
  return params;
}

AddMFEMFESpaceHierarchyAction::AddMFEMFESpaceHierarchyAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMFESpaceHierarchyAction::act()
{
  Moose::MFEM::Problem * mfem_problem = dynamic_cast<Moose::MFEM::Problem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addFESpaceHierarchy(_type, _name, _moose_object_pars);
}

#endif
