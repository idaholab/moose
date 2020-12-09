//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestReuseRaysStudy.h"

registerMooseObject("RayTracingTestApp", TestReuseRaysStudy);

InputParameters
TestReuseRaysStudy::validParams()
{
  auto params = RayTracingStudy::validParams();
  params.set<bool>("_use_ray_registration") = false;
  return params;
}

TestReuseRaysStudy::TestReuseRaysStudy(const InputParameters & parameters)
  : RayTracingStudy(parameters), _executed_once(false)
{
  if (_mesh.dimension() != 1)
    mooseError(_error_prefix, "Works with 1D only");
}

void
TestReuseRaysStudy::generateRays()
{
  if (!_executed_once)
    for (const auto elem : *_mesh.getActiveLocalElementRange())
    {
      auto ray = acquireRay();
      ray->setStart(elem->centroid(), elem);
      ray->setStartingDirection(Point(1, 0, 0));
      moveRayToBuffer(ray);
    }
  else
  {
    for (std::shared_ptr<Ray> & ray : _banked_rays)
    {
      const auto start_point = ray->currentPoint();
      const auto direction = ray->direction();
      const auto elem = ray->currentElem();

      ray->resetCounters();
      ray->clearStartingInfo();
      ray->setStart(start_point, elem);
      ray->setStartingDirection(-direction);
    }
    moveRaysToBuffer(_banked_rays);
    _banked_rays.clear();
  }

  _executed_once = true;
}

void
TestReuseRaysStudy::postExecuteStudy()
{
  _banked_rays = rayBank();
}
