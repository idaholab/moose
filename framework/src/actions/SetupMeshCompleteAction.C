//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupMeshCompleteAction.h"
#include "MooseMesh.h"
#include "Moose.h"
#include "Adaptivity.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<SetupMeshCompleteAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

SetupMeshCompleteAction::SetupMeshCompleteAction(InputParameters params) : Action(params) {}

bool
SetupMeshCompleteAction::completeSetup(MooseMesh * mesh)
{
  bool prepared = mesh->prepared();

  if (!prepared)
    mesh->prepare();

  // Clear the modifiers, they are not used again during the simulation after the mesh has been
  // completed
  _app.clearMeshModifiers();

  return prepared;
}

void
SetupMeshCompleteAction::act()
{
  if (!_mesh)
    mooseError("No mesh file was supplied and no generation block was provided");

  if (_current_task == "execute_mesh_modifiers")
  {
    _app.executeMeshModifiers();
  }
  else if (_current_task == "uniform_refine_mesh")
  {
    /**
     * If possible we'd like to refine the mesh here before the equation systems
     * are setup to avoid doing expensive projections. If however we are doing a
     * file based restart and we need uniform refinements, we'll have to postpone
     * those refinements until after the solution has been read in.
     */
    if (_app.setFileRestart() == false && _app.isRecovering() == false)
    {
      Adaptivity::uniformRefine(_mesh.get());

      if (_displaced_mesh)
        Adaptivity::uniformRefine(_displaced_mesh.get());
    }
  }
  else
  {
    // Prepare the mesh (may occur multiple times)
    completeSetup(_mesh.get());

    if (_displaced_mesh)
      completeSetup(_displaced_mesh.get());
  }
}
