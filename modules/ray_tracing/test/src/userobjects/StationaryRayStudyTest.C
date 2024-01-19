//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StationaryRayStudyTest.h"

registerMooseObject("RayTracingTestApp", StationaryRayStudyTest);

InputParameters
StationaryRayStudyTest::validParams()
{
  auto params = RepeatableRayStudyBase::validParams();

  params.set<bool>("_claim_after_define_rays") = false;
  params.set<bool>("_define_rays_replicated") = false;
  params.set<bool>("_use_ray_registration") = false;

  return params;
}

StationaryRayStudyTest::StationaryRayStudyTest(const InputParameters & parameters)
  : RepeatableRayStudyBase(parameters)
{
}

void
StationaryRayStudyTest::defineRays()
{
  const auto define_ray = [this](const auto elem, const auto & start)
  {
    auto & ray = *_rays.emplace_back(std::move(acquireRay()));
    ray.setStart(start, elem);
    ray.setStationary();
  };

  // elem centroids
  for (const Elem * elem : *_mesh.getActiveLocalElementRange())
    define_ray(elem, elem->vertex_average());
  // boundary side centroids
  for (const auto & bnd_elem : *_mesh.getBoundaryElementRange())
  {
    const Elem * elem = bnd_elem->_elem;
    if (elem->processor_id() == _pid)
      define_ray(elem, elemSide(*elem, bnd_elem->_side).vertex_average());
  }
}
