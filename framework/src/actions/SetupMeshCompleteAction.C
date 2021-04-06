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
#include "TimedPrint.h"

registerMooseAction("MooseApp", SetupMeshCompleteAction, "prepare_mesh");

registerMooseAction("MooseApp",
                    SetupMeshCompleteAction,
                    "delete_remote_elements_after_late_geometric_ghosting");

registerMooseAction("MooseApp", SetupMeshCompleteAction, "uniform_refine_mesh");

registerMooseAction("MooseApp", SetupMeshCompleteAction, "setup_mesh_complete");

defineLegacyParams(SetupMeshCompleteAction);

InputParameters
SetupMeshCompleteAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Perform operations on the mesh in preparation for a simulation.");
  return params;
}

SetupMeshCompleteAction::SetupMeshCompleteAction(InputParameters params)
  : Action(params), _uniform_refine_timer(registerTimedSection("uniformRefine", 2))
{
}

void
SetupMeshCompleteAction::act()
{
  if (!_mesh)
    mooseError("No mesh file was supplied and no generation block was provided");

  if (_current_task == "uniform_refine_mesh")
  {
    // we don't need to run mesh modifiers *again* after they ran already during the mesh
    // splitting process
    if (_app.isUseSplit())
      return;

    // uniform refinement has been done on master, so skip
    if (_app.masterMesh())
      return;

    /**
     * If possible we'd like to refine the mesh here before the equation systems
     * are setup to avoid doing expensive projections. If however we are doing a
     * file based restart and we need uniform refinements, we'll have to postpone
     * those refinements until after the solution has been read in.
     */
    if (_app.getExodusFileRestart() == false && _app.isRecovering() == false)
    {
      TIME_SECTION(_uniform_refine_timer);

      auto & _communicator = *_app.getCommunicator();
      CONSOLE_TIMED_PRINT("Uniformly refining mesh");

      if (_mesh->uniformRefineLevel())
      {
        if (_mesh->meshSubdomains().count(Moose::INTERNAL_SIDE_LOWERD_ID) ||
            _mesh->meshSubdomains().count(Moose::BOUNDARY_SIDE_LOWERD_ID))
          mooseError("HFEM does not support mesh uniform refinement currently.");

        Adaptivity::uniformRefine(_mesh.get());
        // After refinement we need to make sure that all of our MOOSE-specific containers are
        // up-to-date
        _mesh->update();

        if (_displaced_mesh)
        {
          Adaptivity::uniformRefine(_displaced_mesh.get());
          // After refinement we need to make sure that all of our MOOSE-specific containers are
          // up-to-date
          _displaced_mesh->update();
        }
      }
    }
  }
  else if (_current_task == "delete_remote_elements_after_late_geometric_ghosting")
  {
    if (_displaced_mesh &&
        (_mesh->needsRemoteElemDeletion() != _displaced_mesh->needsRemoteElemDeletion()))
      mooseError("Our reference and displaced meshes are not in sync with respect to whether we "
                 "should delete remote elements.");

    // We currently only trigger the needsRemoteDeletion flag if somebody has requested a late
    // geometric ghosting functor and/or we have a displaced mesh
    if (_mesh->needsRemoteElemDeletion())
    {
      _mesh->deleteRemoteElements();
      if (_displaced_mesh)
        _displaced_mesh->deleteRemoteElements();
    }
  }
  else
  {
    // Prepare the mesh (may occur multiple times)
    _mesh->prepare();

    if (_displaced_mesh)
      _displaced_mesh->prepare();
  }
}
