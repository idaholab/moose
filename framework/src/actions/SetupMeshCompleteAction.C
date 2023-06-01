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
#include "FEProblemBase.h"

registerMooseAction("MooseApp", SetupMeshCompleteAction, "prepare_mesh");

registerMooseAction("MooseApp",
                    SetupMeshCompleteAction,
                    "delete_remote_elements_after_late_geometric_ghosting");

registerMooseAction("MooseApp", SetupMeshCompleteAction, "uniform_refine_mesh");

registerMooseAction("MooseApp", SetupMeshCompleteAction, "setup_mesh_complete");

InputParameters
SetupMeshCompleteAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Perform operations on the mesh in preparation for a simulation.");
  return params;
}

SetupMeshCompleteAction::SetupMeshCompleteAction(const InputParameters & params) : Action(params) {}

void
SetupMeshCompleteAction::act()
{
  if (!_mesh)
    mooseError("No mesh file was supplied and no generation block was provided");

  if (_current_task == "uniform_refine_mesh")
  {
    // we don't need to run mesh modifiers *again* after they ran already during the mesh
    // splitting process
    // A uniform refinement is helpful for some instances when using a pre-split mesh.
    // For example, a 'coarse' mesh might completely resolve geometry (also is large)
    // but does not have enough resolution for the interior. For this scenario,
    // we pre-split the coarse mesh, and load the pre-split mesh in parallel,
    // and then do a few levels of uniform refinements to have a fine mesh that
    // potentially resolves physics features.
    if (_mesh->isSplit() && _mesh->skipRefineWhenUseSplit())
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
    if (_app.getExodusFileRestart() == false)
    {
      if (_app.isRecovering() == false || !_app.isUltimateMaster())
      {
        TIME_SECTION("uniformRefine", 2, "Uniformly Refining");

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
  }
  else if (_current_task == "delete_remote_elements_after_late_geometric_ghosting")
  {
    TIME_SECTION("deleteRemoteElems", 2, "Deleting Remote Elements");

    if (_displaced_mesh &&
        (_mesh->needsRemoteElemDeletion() != _displaced_mesh->needsRemoteElemDeletion()))
      mooseError("Our reference and displaced meshes are not in sync with respect to whether we "
                 "should delete remote elements.");

    // We currently only trigger the needsRemoteDeletion flag if somebody has requested a late
    // geometric ghosting functor and/or we have a displaced mesh
    if (_mesh->needsRemoteElemDeletion())
    {
      // Must make sure to create the mortar meshes to build our data structures that ensure we will
      // keep the correct elements in the mesh around
      _problem->updateMortarMesh();
      _mesh->deleteRemoteElements();
      if (_displaced_mesh)
        _displaced_mesh->deleteRemoteElements();
    }
  }
  else
  {
    // Prepare the mesh (may occur multiple times)
    bool prepare_for_use_called_on_undisplaced = false;
    {
      TIME_SECTION("completeSetupUndisplaced", 2, "Setting Up Undisplaced Mesh");
      prepare_for_use_called_on_undisplaced = _mesh->prepare(/*mesh_to_clone=*/nullptr);
    }

    if (_displaced_mesh)
    {
      TIME_SECTION("completeSetupDisplaced", 2, "Setting Up Displaced Mesh");
      // If the reference mesh was prepared, then we must prepare also
      _displaced_mesh->prepare(
          /*mesh_to_clone=*/prepare_for_use_called_on_undisplaced ? &_mesh->getMesh() : nullptr);
    }
  }
}
