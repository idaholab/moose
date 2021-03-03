//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayBoundaryConditionBase.h"

// Local includes
#include "RayTracingStudy.h"
#include "TraceRay.h"

// MOOSE includes
#include "Assembly.h"

InputParameters
RayBoundaryConditionBase::validParams()
{
  auto params = RayTracingObject::validParams();
  params += BoundaryRestrictableRequired::validParams();

  params.addParam<std::vector<std::string>>("depends_on",
                                            "Other RayBCs that this RayBC depends on");

  params.registerBase("RayBoundaryCondition");
  params.registerSystemAttributeName("RayBoundaryCondition");

  // We don't currently allow reinits on RayBCs just yet
  params.suppressParameter<bool>("implicit");

  return params;
}

RayBoundaryConditionBase::RayBoundaryConditionBase(const InputParameters & params)
  : RayTracingObject(params),
    Restartable(this, "RayBoundaryConditions"),
    BoundaryRestrictableRequired(this, false), // false for sidesets
    _current_intersection_point(_study.traceRay(_tid).currentIntersectionPoint()),
    _current_bnd_id(_study.traceRay(_tid).currentBoundaryID())
{
  // Add dependencies
  if (params.isParamSetByUser("depends_on"))
    for (const auto & name : getParam<std::vector<std::string>>("depends_on"))
      dependsOn(name);
}

RayBoundaryConditionBase::~RayBoundaryConditionBase() {}

void
RayBoundaryConditionBase::changeRayDirection(const Point & direction, const bool skip_changed_check)
{
  if (!_current_ray->shouldContinue())
    mooseError("Cannot changeRayDirection() for a Ray that should not continue\n\n",
               _current_ray->getInfo());

  if (!skip_changed_check && _current_ray->trajectoryChanged())
    mooseError("Cannot change direction for a ray whose direction has already been changed\n\n",
               _current_ray->getInfo());

  if (_current_ray->endSet())
    mooseError("Cannot change the direction of a Ray whose end point is set upon generation "
               "(via setStartingEndPoint()).\n\n",
               _current_ray->getInfo());

  _current_ray->changeDirection(direction, Ray::ChangeDirectionKey());
}

MooseUtils::SharedPool<Ray>::PtrType
RayBoundaryConditionBase::acquireRay(const Point & direction)
{
  mooseAssert(_study.currentlyPropagating(), "Should not be getting a Ray while not propagating");

  // Acquire a Ray with the proper sizes and a unique ID, and set the start info
  auto ray = _study.acquireRayDuringTrace(_tid, RayTracingStudy::AcquireMoveDuringTraceKey());
  ray->setStart(_current_intersection_point, _current_elem, _current_intersected_side);
  ray->setStartingDirection(direction);

  return ray;
}

void
RayBoundaryConditionBase::moveRayToBuffer(MooseUtils::SharedPool<Ray>::PtrType && ray)
{
  mooseAssert(_study.currentlyPropagating(),
              "Should not move Rays into buffer while not propagating");

  _study.moveRayToBufferDuringTrace(
      std::move(ray), _tid, RayTracingStudy::AcquireMoveDuringTraceKey());
}
