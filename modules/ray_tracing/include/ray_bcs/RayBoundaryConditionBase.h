//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local Includes
#include "RayTracingObject.h"
#include "Ray.h"

// MOOSE Includes
#include "Restartable.h"
#include "BoundaryRestrictableRequired.h"

/**
 * Base class for the RayBC syntax
 *
 * Operates on Rays when they hit a boundary
 */
class RayBoundaryConditionBase : public RayTracingObject,
                                 public Restartable,
                                 public BoundaryRestrictableRequired
{
public:
  RayBoundaryConditionBase(const InputParameters & params);
  virtual ~RayBoundaryConditionBase();

  static InputParameters validParams();

  /**
   * Called on a Ray on the boundary to apply the Ray boundary condition.
   *
   * Important information available during onBoundary():
   *   currentRay() - The current Ray that is being applied at the boundary
   *   _current_elem - The current Elem the Ray is tracing in
   *   _current_intersected_side - The side intersected on the boundary
   *   _current_intersection_point - The point intersected on the boundary
   *   _current_intersected_extrema - The extrema (vertex/edge) intersected on
   *                                  _current_elem on the boundary, if any
   *   _current_bnd_id - The ID of the intersected boundary
   *
   * The only positional modification you can make to the Ray is to modify its
   * direction via changeRayDirection().
   *
   * Note on \p num_applying: If hitting an edge in 3D or a vertex in 2D,
   * the same Ray boundary condition may be applied multiple times if the Ray
   * boundary condition is defined on multiple boundaries that meet at said
   * edge/vertex. This will identify when this occurs.
   */
  virtual void onBoundary(const unsigned int num_applying) = 0;

protected:
  /**
   * Changes the current Ray's direction.
   *
   * The changed direction must be incoming on _current_intersected_side of _current_elem.
   *
   * @param direction The direction to change to
   * @param skip_changed_check Whether or not to skip the check if another BC has just changed this
   * Ray's trajectory
   */
  void changeRayDirection(const Point & direction, const bool skip_changed_check = false);

  /**
   * Acquires a Ray to be used for generating a new Ray while tracing on the boundary.
   *
   * Appropriately sets the information needed:
   * - Sizes the Ray's data to the size needed by the study
   * - Sets the starting elem to the current element
   * - Sets the incoming side to the current intersected side
   * - Sets the starting point to the current intersection point
   * - Sets the direction as provided by \p direction
   * - Sets the RayID to something unique
   */
  std::shared_ptr<Ray> acquireRay(const Point & direction);

  /**
   * Moves a Ray into the working buffer to be traced during tracing with a
   * meaningful error on verification failure
   */
  void moveRayToBuffer(std::shared_ptr<Ray> & ray);

  /// The current intersection point on the boundary
  const Point & _current_intersection_point;
  /// The ID of the current intersected boundary
  const BoundaryID & _current_bnd_id;
};
