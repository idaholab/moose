//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "SetMFEMMeshFESpaceAction.h"

registerMooseAction("MooseApp", SetMFEMMeshFESpaceAction, "set_mesh_fe_space");

InputParameters
SetMFEMMeshFESpaceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set the mesh nodal finite element space to the same as the mesh "
                             "displacement variable, if one is specified.");
  return params;
}

SetMFEMMeshFESpaceAction::SetMFEMMeshFESpaceAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
SetMFEMMeshFESpaceAction::act()
{
  auto * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());

  if (!mfem_problem)
  {
    return;
  }

  mfem::ParMesh & mesh = mfem_problem->mesh().getMFEMParMesh();
  auto const displacement = mfem_problem->getMeshDisplacementGridFunction();
  if (!displacement)
  {
    return;
  }

  mesh.SetNodalFESpace(displacement.value().get().ParFESpace());
}

#endif
