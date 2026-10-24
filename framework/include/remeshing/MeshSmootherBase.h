//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseTypes.h"
#include "Remeshing.h"
#include "SetupInterface.h"

class FEProblemBase;
class MooseMesh;

/**
 * Base class for the mesh motion of the Remeshing engine.
 *
 * A smoother owns the pseudo-displacement d that the engine adds to the reference coordinates X0
 * snapshotted at the last remesh. updatePseudoDisplacement() writes the total displacement since
 * that snapshot, not an increment: the engine sets the coordinates to x = X0 + d from scratch every
 * step, so that resetting d to zero at a remesh event is exact.
 *
 * The class is named MeshSmootherBase, and not MeshSmoother, because libMesh already defines
 * MeshSmoother and MOOSE source files pull the libMesh namespace in.
 */
class MeshSmootherBase : public MooseObject, public SetupInterface
{
public:
  static InputParameters validParams();

  MeshSmootherBase(const InputParameters & parameters);

  /**
   * Update the total pseudo-displacement d since the last remesh, in place in
   * pseudoDisplacement().
   *
   * @param dt the time step size the executioner is about to solve with
   */
  virtual void updatePseudoDisplacement(Real dt) = 0;

  /**
   * Drop the state that a change of mesh topology invalidates.
   *
   * Called by the engine at a remesh event, after the transfer has read the old solution and
   * before any replaced entity is deleted, so handles into them are still valid here. The engine
   * resets X0 and zeroes d itself, so this only has to deal with state the smoother owns.
   */
  virtual void reset() {}

  /**
   * Rebuild any internal system on the new mesh.
   *
   * Called by the engine at a remesh event, after FEProblemBase::meshChanged() has reinitialized
   * the equation systems.
   */
  virtual void reinitOnNewMesh() {}

protected:
  /**
   * The total pseudo-displacement d since the last remesh, keyed by node id. This is the container
   * updatePseudoDisplacement() writes into.
   */
  Remeshing::PointMap & pseudoDisplacement();

  /**
   * The reference node coordinates X0 snapshotted at the last remesh, keyed by node id.
   */
  const Remeshing::PointMap & referenceCoordinates() const;

  /**
   * Set the coordinates of every node this rank holds to x = X0 + d, from referenceCoordinates()
   * and pseudoDisplacement().
   *
   * Unlike Remeshing::applyPseudoDisplacement(), this leaves the cached geometry and the displaced
   * mesh alone: a smoother places the nodes for its own use, and the engine refreshes everything
   * once after updatePseudoDisplacement() returns.
   */
  void placeNodesAtPseudoDisplacement();

  /// The problem being remeshed
  FEProblemBase & _fe_problem;

  /// The reference mesh
  MooseMesh & _mesh;

  /// The engine driving this smoother
  Remeshing & _remeshing;
};
