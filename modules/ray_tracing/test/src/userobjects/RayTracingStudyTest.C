//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingStudyTest.h"

#include "libmesh/parallel_sync.h"

registerMooseObject("RayTracingTestApp", RayTracingStudyTest);

InputParameters
RayTracingStudyTest::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addParam<bool>("add_duplicate_ray", "Add a duplicate Ray to the buffer");
  params.addParam<bool>("add_local_non_unique_id_ray",
                        "Add two Rays on each processor with the same ID");
  params.addParam<bool>("add_global_non_unique_id_ray",
                        "Add a Ray on each processor with the same ID");

  params.set<bool>("_use_ray_registration") = false;

  return params;
}

RayTracingStudyTest::RayTracingStudyTest(const InputParameters & parameters)
  : RayTracingStudy(parameters)
{
}

void
RayTracingStudyTest::generateRays()
{
  mooseAssert(_mesh.getMesh().n_local_elem(), "Local mesh does not have an element");
  const Elem * elem = *_mesh.getActiveLocalElementRange()->begin();

  auto ray = acquireRay();
  ray->setStart(elem->centroid());
  ray->setStartingEndPoint(elem->point(0));

  if (parameters().isParamSetByUser("add_duplicate_ray") && getParam<bool>("add_duplicate_ray"))
  {
    std::shared_ptr<Ray> duplicate_ray = ray;
    moveRayToBuffer(ray);
    moveRayToBuffer(duplicate_ray);
  }

  if (parameters().isParamSetByUser("add_local_non_unique_id_ray") &&
      getParam<bool>("add_local_non_unique_id_ray"))
  {
    std::shared_ptr<Ray> other_ray = acquireCopiedRay(*ray);
    moveRayToBuffer(ray);
    moveRayToBuffer(other_ray);
  }

  if (parameters().isParamSetByUser("add_global_non_unique_id_ray") &&
      getParam<bool>("add_global_non_unique_id_ray"))
  {
    mooseAssert(n_processors() > 1, "Needs multiple ranks");

    std::map<processor_id_type, std::vector<std::shared_ptr<Ray>>> send_map;
    if (_pid == 0)
    {
      for (processor_id_type pid = 1; pid < n_processors(); ++pid)
        send_map[pid].push_back(ray);
      moveRayToBuffer(ray);
    }

    auto add_ray_functor = [&](processor_id_type, const std::vector<std::shared_ptr<Ray>> & rays) {
      std::shared_ptr<Ray> ray = rays[0];
      ray->clearStartingInfo();
      ray->setStart(elem->centroid());
      ray->setStartingEndPoint(elem->point(0));
      moveRayToBuffer(ray);
    };

    Parallel::push_parallel_packed_range(
        comm(), send_map, (RayTracingStudy *)this, add_ray_functor);
  }
}
