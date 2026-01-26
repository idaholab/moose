//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMDGBoundaryConditions.h"

registerMooseAction("MooseApp", AddMFEMDGBoundaryConditions, "add_mfem_dg_bc");

InputParameters
AddMFEMDGBoundaryConditions::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add in DG Kernel on the boundary");
  return params;
}

AddMFEMDGBoundaryConditions::AddMFEMDGBoundaryConditions(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMDGBoundaryConditions::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addDGBoundaryCondition(_type, _name, _moose_object_pars);
}

#endif
