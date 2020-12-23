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

// MOOSE includes
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "BlockRestrictable.h"
#include "RandomInterface.h"
#include "MaterialPropertyInterface.h"
#include "Restartable.h"

/**
 * Base object for the RayKernel syntax
 *
 * Operates on Ray segments
 */
class RayKernelBase : public RayTracingObject,
                      public CoupleableMooseVariableDependencyIntermediateInterface,
                      public BlockRestrictable,
                      public RandomInterface,
                      public MaterialPropertyInterface,
                      public Restartable
{
public:
  RayKernelBase(const InputParameters & params);
  virtual ~RayKernelBase();

  static InputParameters validParams();

  /**
   * Called on each segment of a Ray
   *
   * Important information available during onSegment():
   *   currentRay() - The current Ray that is being traced on the segment
   *   _current_elem - The current Elem the Ray is tracing in
   *   _current_segment_start - The start point of the segment
   *   _current_segment_end - The end point of the segment
   *   _current_segment_length - The length of the segment
   *   _current_intersected_side - The side intersected on _current_elem (if any)
   *   _current_intersected_extrema - The extrema (vertex/edge) intersected on
   *                                  _current_elem (if any)
   *   _current_incoming_side - The side of _current_elem that _current_segment_start
   *                            is on (if any)
   */
  virtual void onSegment() = 0;

  /**
   * Called at the beginning of the trace on this processor/thread for a Ray
   *
   * This is not only called at the _true_ beginning of the Ray, but instead
   * it is called every time this processor/thread starts the trace for the
   * part it can trace
   */
  virtual void preTrace();

  /**
   * Whether or not this RayKernel needs a segment reinit
   */
  bool needSegmentReinit() const { return _need_segment_reinit; }

protected:
  /**
   * Changes the current Ray's start point and direction.
   *
   * The start point must be within the current element.
   */
  void changeRayStartDirection(const Point & start, const Point & direction);

  /**
   * Acquires a Ray to be used for generating a new Ray while tracing on the boundary.
   *
   * Appropriately sets the information needed:
   * - Sizes the Ray's data to the size needed by the study
   * - Sets the starting elem to the current element
   * - Sets the start point to \p start
   * - Sets the direction as provided by \p direction
   * - Sets the RayID to something unique
   */
  std::shared_ptr<Ray> acquireRay(const Point & start, const Point & direction);

  /**
   * Moves a Ray into the working buffer to be traced during tracing with a
   * meaningful error on verification failure
   */
  void moveRayToBuffer(std::shared_ptr<Ray> & ray);

  /// The start point of the current Ray's segment
  const Point & _current_segment_start;
  /// The end point of the current Ray's segment
  const Point & _current_segment_end;
  /// The length of the current Ray's segment
  const Real & _current_segment_length;
  /// The current side of _current_elem that _current_segment_start is on (if any)
  const unsigned short & _current_incoming_side;

  /// Whether or not this RayKernel needs a segment reinit
  const bool _need_segment_reinit;
};

#define usingRayKernelBaseMembers                                                                  \
  usingRayTracingObjectMembers;                                                                    \
  usingBlockRestrictableMembers;                                                                   \
  using RayKernelBase::changeRayStartDirection;                                                    \
  using RayKernelBase::acquireRay;                                                                 \
  using RayKernelBase::moveRayToBuffer
