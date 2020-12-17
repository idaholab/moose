//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayKernelBase.h"

// Local includes
#include "RayTracingStudy.h"
#include "TraceRay.h"

InputParameters
RayKernelBase::validParams()
{
  auto params = RayTracingObject::validParams();
  params += BlockRestrictable::validParams();
  params += RandomInterface::validParams();
  params += MaterialPropertyInterface::validParams();

  params.addParam<std::vector<std::string>>("depends_on",
                                            "Other RayKernels that this RayKernel depends on");

  params.registerBase("RayKernel");
  params.registerSystemAttributeName("RayKernel");

  // Allows for a RayKernel to request that it needs a reinit on its segment
  // We have this so that for RayKernels that still need qps and weights but do not
  // have any active variables or materials can still call reinitSegment()
  params.addPrivateParam<bool>("_need_segment_reinit", false);

  return params;
}

RayKernelBase::RayKernelBase(const InputParameters & params)
  : RayTracingObject(params),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    BlockRestrictable(this),
    RandomInterface(params, _fe_problem, _tid, false),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    Restartable(this, "RayKernels"),
    _current_segment_start(_study.traceRay(_tid).currentIncomingPoint()),
    _current_segment_end(_study.traceRay(_tid).currentIntersectionPoint()),
    _current_segment_length(_study.traceRay(_tid).currentIntersectionDistance()),
    _current_incoming_side(_study.traceRay(_tid).currentIncomingSide()),
    _need_segment_reinit(getParam<bool>("_need_segment_reinit"))
{
  // Add dependencies
  if (params.isParamSetByUser("depends_on"))
    for (const auto & name : getParam<std::vector<std::string>>("depends_on"))
      dependsOn(name);

  // Stateful material properties are not allowed on RayKernels
  statefulPropertiesAllowed(false);
}

RayKernelBase::~RayKernelBase() {}

void
RayKernelBase::changeRayStartDirection(const Point & start, const Point & direction)
{
  mooseAssert(_study.currentlyPropagating(), "Should not change Ray outside of tracing");

  const auto & ray = currentRay();

  if (!ray->shouldContinue())
  {
    if (ray->endSet() && ray->atEnd())
      mooseError(_error_prefix,
                 ": Cannot changeRayStartDirection() for a Ray that should not continue.\n\n",
                 "It has also hit its user-set end point.\n\n",
                 ray->getInfo());
    else
      mooseError(_error_prefix,
                 ": Cannot changeRayStartDirection() for a Ray that should not continue.\n\n",
                 ray->getInfo());
  }

  if (!ray->intersections())
    mooseError(_error_prefix, ": Cannot changeRayStartDirection() for Ray that has not moved");

  if (ray->trajectoryChanged())
    mooseError(
        _error_prefix,
        " is trying to change a Ray's trajectory, but its trajectory has already been changed\n\n",
        ray->getInfo());

  if (ray->endSet())
    mooseError(_error_prefix,
               " is trying to change the direction of a Ray that"
               "\nhad its end point set upon generation (via setStartingEndPoint())."
               "\n\nThis is not currently supported.\n\n",
               ray->getInfo());

  if (_study.verifyRays() && !_current_elem->contains_point(start))
    mooseError(_error_prefix,
               " is trying to change a Ray's trajectory,\n",
               "but the new start point is not within the Ray's current element\n\n",
               ray->getInfo());

  ray->changeStartDirection(start, direction, Ray::ChangeStartDirectionKey());
}

std::shared_ptr<Ray>
RayKernelBase::acquireRay(const Point & start, const Point & direction)
{
  mooseAssert(_study.currentlyPropagating(), "Should not be getting a Ray while not propagating");

  // Acquire a Ray with the proper sizes and a unique ID, and set the start info
  std::shared_ptr<Ray> ray =
      _study.acquireRayDuringTrace(_tid, RayTracingStudy::AcquireMoveDuringTraceKey());
  ray->setStart(start, _current_elem, RayTracingCommon::invalid_side);
  ray->setStartingDirection(direction);

  return ray;
}

void
RayKernelBase::moveRayToBuffer(std::shared_ptr<Ray> & ray)
{
  mooseAssert(_study.currentlyPropagating(),
              "Should not move Rays into buffer while not propagating");

  if (_current_elem != ray->currentElem())
    mooseError(_error_prefix,
               ": A Ray was added to the buffer mid-trace that does not\n",
               "start in the same Elem as the ",
               type(),
               " that created it\n\n",
               currentRay()->getInfo());

  _study.moveRayToBufferDuringTrace(ray, _tid, RayTracingStudy::AcquireMoveDuringTraceKey());
}

void
RayKernelBase::preTrace()
{
}
