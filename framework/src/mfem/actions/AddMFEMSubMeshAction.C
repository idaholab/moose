//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "AddMFEMSubMeshAction.h"

registerMooseAction("MooseApp", AddMFEMSubMeshAction, "add_mfem_submeshes");

InputParameters
AddMFEMSubMeshAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM SubMesh object to the simulation.");
  return params;
}

AddMFEMSubMeshAction::AddMFEMSubMeshAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMSubMeshAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addSubMesh(_type, _name, _moose_object_pars);
  else
    mooseError("Cannot add SubMeshes unless an MFEMProblem is in use.");
}

#endif
