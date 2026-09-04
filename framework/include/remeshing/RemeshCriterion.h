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
 * Base class for the tests that decide when the Remeshing engine remeshes.
 *
 * shouldRemesh() is evaluated on the current configuration x = X0 + d, or on the displaced
 * configuration when the problem has true displacements.
 *
 * Parallel contract: shouldRemesh() must return the same value on every rank. The implementation
 * owns that, by reducing its measured quantity over the communicator before comparing it to its
 * threshold; minimumBelowThreshold() and maximumAboveThreshold() do that reduction. The engine
 * reduces only the boolean over the criteria and over the ranks, which makes a firing criterion
 * fire everywhere but cannot repair a rank local decision.
 */
class RemeshCriterion : public MooseObject, public SetupInterface
{
public:
  static InputParameters validParams();

  RemeshCriterion(const InputParameters & parameters);

  /**
   * @return whether the mesh has to be remeshed now, the same value on every rank
   */
  virtual bool shouldRemesh() = 0;

  /**
   * @return whether shouldRemesh() reads the field of an Adaptivity Indicator, which the engine
   * recomputes on the current mesh before it evaluates the criteria
   */
  virtual bool consumesIndicators() const { return false; }

protected:
  /**
   * Reduce \p local_value with a parallel minimum and compare it to \p threshold.
   *
   * @param local_value the extremum of the measured quantity over this rank
   * @param threshold the value the quantity is not allowed to fall below
   * @return whether the smallest value over all ranks is below \p threshold
   */
  bool minimumBelowThreshold(Real local_value, Real threshold) const;

  /**
   * Reduce \p local_value with a parallel maximum and compare it to \p threshold.
   *
   * @param local_value the extremum of the measured quantity over this rank
   * @param threshold the value the quantity is not allowed to exceed
   * @return whether the largest value over all ranks is above \p threshold
   */
  bool maximumAboveThreshold(Real local_value, Real threshold) const;

  /**
   * The mesh the criterion is evaluated on: the displaced mesh when the problem has displacement
   * variables, otherwise the reference mesh, which carries the current configuration x = X0 + d.
   *
   * The mesh is selected on every evaluation rather than cached, because the engine is told the
   * displacement variables by the RemeshingAction, which does not have to run before this object
   * is constructed.
   */
  MooseMesh & evaluationMesh() const;

  /**
   * Error out when the [Remeshing] block does not set mesh_movement = true, because the
   * pseudo-displacement is then identically zero and any criterion built on it is meaningless.
   */
  void requireMeshMovement() const;

  /**
   * The total pseudo-displacement d accumulated since the last remesh, keyed by node id.
   *
   * Errors through requireMeshMovement() when the [Remeshing] block does not set
   * mesh_movement = true.
   */
  const Remeshing::PointMap & pseudoDisplacement() const;

  /// The problem being remeshed
  FEProblemBase & _fe_problem;

  /// The reference mesh
  MooseMesh & _mesh;

  /// The engine driving this criterion
  const Remeshing & _remeshing;
};
