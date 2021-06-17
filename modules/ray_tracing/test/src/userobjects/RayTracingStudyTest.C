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

  params.addParam<bool>("add_duplicate_ray", false, "Add a duplicate Ray to the buffer");
  params.addParam<bool>(
      "add_local_non_unique_id_ray", false, "Add two Rays on each processor with the same ID");
  params.addParam<bool>(
      "add_global_non_unique_id_ray", false, "Add a Ray on each processor with the same ID");

  params.addParam<bool>(
      "ray_at_end_without_set", false, "Test Ray::atEnd() without the end being set");
  params.addParam<bool>(
      "ray_end_point_without_set", false, "Test Ray::endPoint() without the end being set");
  params.addParam<bool>(
      "ray_set_start_again", false, "Test setting a Ray's start point multiple times");
  params.addParam<bool>("ray_set_start_fail_bbox",
                        false,
                        "Test setting a Ray's start point to a point that fails the bbox check");
  params.addParam<bool>("ray_set_side_without_elem",
                        false,
                        "Test setting a Ray's incoming side without a starting element");
  params.addParam<bool>(
      "ray_set_invalid_side", false, "Test setting a Ray's incoming side to an invalid one");
  params.addParam<bool>(
      "ray_set_bad_side", false, "Test setting a Ray's incoming side to a wrong side");
  params.addParam<bool>(
      "ray_set_bad_start",
      false,
      "Test setting a Ray's start point to one that is not within the starting element");
  params.addParam<bool>("ray_set_direction_before_start",
                        false,
                        "Test setting a Ray's direction before its start point");
  params.addParam<bool>(
      "ray_set_direction_again", false, "Test setting a Ray's direction multiple times");
  params.addParam<bool>(
      "ray_set_zero_direction", false, "Test setting a Ray's direction to the zero vector");
  params.addParam<bool>(
      "ray_set_end_before_start", false, "Test setting a Ray's end point before its start point");
  params.addParam<bool>(
      "ray_set_end_equal_start", false, "Test setting a Ray's end point to its start point");
  params.addParam<bool>("ray_set_end_with_direction",
                        false,
                        "Test setting a Ray's end point after setting its direction");
  params.addParam<bool>("ray_set_distance_with_end",
                        false,
                        "Test setting a Ray's max distance after setting its end point");
  params.addParam<bool>("ray_set_end_with_distance",
                        false,
                        "Test setting a Ray's end point after setting its max distance");
  params.addParam<bool>("ray_set_end_fail_bbox",
                        false,
                        "Test setting a Ray's end point to a point that fails the bbox check");
  params.addParam<bool>("ray_set_distance_before_start",
                        false,
                        "Test setting a Ray's max distance before its start point");
  params.addParam<bool>(
      "ray_set_distance_negative", false, "Test setting a Ray's max distance to a negative value");
  params.addParam<bool>("ray_set_distance_with_end",
                        false,
                        "Test setting a Ray's max distance after its end point has been set");

  params.addParam<bool>("ray_error_if_tracing", false, "Tests Ray::errorIfTracing()");
  params.addParam<bool>(
      "ray_reset_counters", false, "Tests resetting a Ray's counters after it has began tracing");

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

  if (getParam<bool>("ray_set_direction_before_start"))
    ray->setStartingDirection(Point(1, 0, 0));
  if (getParam<bool>("ray_set_end_before_start"))
    ray->setStartingEndPoint(elem->point(0));
  if (getParam<bool>("ray_set_distance_before_start"))
    ray->setStartingMaxDistance(1);

  if (getParam<bool>("ray_set_side_without_elem"))
    ray->setStart(elem->centroid(), nullptr, 0);
  else if (getParam<bool>("ray_set_invalid_side"))
    ray->setStart(elem->centroid(), elem, 100);
  else if (getParam<bool>("ray_set_bad_side"))
    ray->setStart(elem->centroid(), elem, 0);
  else if (getParam<bool>("ray_set_start_fail_bbox"))
    ray->setStart(Point(1e6, 1e6, 1e6));
  else if (getParam<bool>("ray_set_bad_start"))
  {
    const Elem * another_elem = elem->neighbor_ptr(0);
    if (!another_elem)
      another_elem = elem->neighbor_ptr(1);
    ray->setStart(elem->centroid(), another_elem);
  }
  else
    ray->setStart(elem->centroid(), elem);

  if (getParam<bool>("ray_set_end_with_distance"))
  {
    ray->setStartingMaxDistance(1);
    ray->setStartingEndPoint(elem->point(0));
  }

  if (!getParam<bool>("ray_at_end_without_set") && !getParam<bool>("ray_end_point_without_set") &&
      !getParam<bool>("ray_set_direction_again") && !getParam<bool>("ray_set_zero_direction") &&
      !getParam<bool>("ray_set_end_equal_start") && !getParam<bool>("ray_set_end_with_direction") &&
      !getParam<bool>("ray_set_end_fail_bbox"))
    ray->setStartingEndPoint(elem->point(0));

  if (getParam<bool>("ray_set_distance_with_end"))
    ray->setStartingMaxDistance(1);
  if (getParam<bool>("ray_set_end_equal_start"))
    ray->setStartingEndPoint(ray->currentPoint());
  if (getParam<bool>("ray_set_end_fail_bbox"))
    ray->setStartingEndPoint(Point(1e6, 1e6, 1e6));

  if (getParam<bool>("ray_set_direction_again"))
  {
    ray->setStartingDirection(Point(1, 0, 0));
    ray->setStartingDirection(Point(-1, 0, 0));
  }
  if (getParam<bool>("ray_set_zero_direction"))
    ray->setStartingDirection(Point(0, 0, 0));
  if (getParam<bool>("ray_set_end_with_direction"))
  {
    ray->setStartingDirection(Point(1, 0, 0));
    ray->setStartingEndPoint(elem->point(0));
  }

  if (getParam<bool>("ray_set_distance_negative"))
    ray->setStartingMaxDistance(-1);
  if (getParam<bool>("ray_set_distance_with_end"))
    ray->setStartingMaxDistance(1);

  if (getParam<bool>("add_duplicate_ray"))
  {
    std::shared_ptr<Ray> duplicate_ray = ray;
    moveRayToBuffer(ray);
    moveRayToBuffer(duplicate_ray);
  }

  if (getParam<bool>("add_local_non_unique_id_ray"))
  {
    std::shared_ptr<Ray> other_ray = acquireCopiedRay(*ray);
    moveRayToBuffer(ray);
    moveRayToBuffer(other_ray);
  }

  if (getParam<bool>("add_global_non_unique_id_ray"))
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

    Parallel::push_parallel_packed_range(comm(), send_map, parallelStudy(), add_ray_functor);
  }

  if (getParam<bool>("ray_at_end_without_set"))
    _console << ray->atEnd() << std::endl;

  if (getParam<bool>("ray_end_point_without_set"))
    _console << ray->endPoint() << std::endl;

  if (getParam<bool>("ray_set_start_again"))
    ray->setStart(1.01 * elem->centroid());

  if (getParam<bool>("ray_error_if_tracing") || getParam<bool>("ray_reset_counters"))
    moveRayToBuffer(ray);
}

void
RayTracingStudyTest::postExecuteStudy()
{
  if (getParam<bool>("ray_error_if_tracing"))
    for (auto & ray : rayBank())
      ray->clearStartingInfo();

  if (getParam<bool>("ray_reset_counters"))
    for (auto & ray : rayBank())
      ray->resetCounters();
}
