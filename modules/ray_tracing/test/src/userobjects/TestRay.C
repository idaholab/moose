//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestRay.h"

registerMooseObject("RayTracingTestApp", TestRay);

InputParameters
TestRay::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addParam<bool>("at_end_without_set", false, "Test Ray::atEnd() without the end being set");
  params.addParam<bool>(
      "end_point_without_set", false, "Test Ray::endPoint() without the end being set");
  params.addParam<bool>(
      "set_start_again", false, "Test setting a Ray's start point multiple times");
  params.addParam<bool>(
      "set_direction_again", false, "Test setting a Ray's direction multiple times");
  params.addParam<bool>("set_start_fail_bbox",
                        false,
                        "Test setting a Ray's start point to a point that fails the bbox check");
  params.addParam<bool>("set_side_without_elem",
                        false,
                        "Test setting a Ray's incoming side without a starting element");
  params.addParam<bool>(
      "set_invalid_side", false, "Test setting a Ray's incoming side to an invalid one");
  params.addParam<bool>(
      "set_bad_side", false, "Test setting a Ray's incoming side to a wrong side");
  params.addParam<bool>(
      "set_bad_start",
      false,
      "Test setting a Ray's start point to one that is not within the starting element");
  params.addParam<bool>(
      "set_direction_before_start", false, "Test setting a Ray's direction before its start point");
  params.addParam<bool>(
      "set_zero_direction", false, "Test setting a Ray's direction to the zero vector");
  params.addParam<bool>(
      "set_end_before_start", false, "Test setting a Ray's end point before its start point");
  params.addParam<bool>(
      "set_end_equal_start", false, "Test setting a Ray's end point to its start point");
  params.addParam<bool>("set_end_with_direction",
                        false,
                        "Test setting a Ray's end point after setting its direction");
  params.addParam<bool>("set_distance_with_end",
                        false,
                        "Test setting a Ray's max distance after setting its end point");
  params.addParam<bool>("set_end_with_distance",
                        false,
                        "Test setting a Ray's end point after setting its max distance");
  params.addParam<bool>("set_end_fail_bbox",
                        false,
                        "Test setting a Ray's end point to a point that fails the bbox check");
  params.addParam<bool>("set_distance_before_start",
                        false,
                        "Test setting a Ray's max distance before its start point");
  params.addParam<bool>(
      "set_distance_negative", false, "Test setting a Ray's max distance to a negative value");
  params.addParam<bool>(
      "set_start_inactive", false, "Tests setting a Ray's starting element to an inactive element");

  params.set<bool>("_use_ray_registration") = false;

  return params;
}

TestRay::TestRay(const InputParameters & parameters) : RayTracingStudy(parameters) {}

void
TestRay::generateRays()
{
  if (_mesh.getMesh().n_local_elem() != 0)
  {
    const Elem * elem = *_mesh.getActiveLocalElementRange()->begin();

    auto ray = acquireRay();

    if (getParam<bool>("set_distance_before_start"))
      ray->setStartingMaxDistance(1);
    if (getParam<bool>("at_end_without_set"))
      ray->atEnd();
    if (getParam<bool>("end_point_without_set"))
      ray->endPoint();
    if (getParam<bool>("set_start_fail_bbox"))
      ray->setStart(Point(1e6, 1e6, 1e6));
    if (getParam<bool>("set_side_without_elem"))
      ray->setStart(elem->vertex_average(), nullptr, 0);
    if (getParam<bool>("set_invalid_side"))
      ray->setStart(elem->vertex_average(), elem, 100);
    if (getParam<bool>("set_bad_side"))
      ray->setStart(elem->vertex_average(), elem, 0);
    if (getParam<bool>("set_direction_before_start"))
      ray->setStartingDirection(Point(1, 0, 0));
    if (getParam<bool>("set_bad_start"))
    {
      const Elem * another_elem = elem->neighbor_ptr(0);
      if (!another_elem)
        another_elem = elem->neighbor_ptr(1);
      ray->setStart(elem->vertex_average(), another_elem);
    }
    if (getParam<bool>("set_end_before_start"))
      ray->setStartingEndPoint(elem->point(0));
    if (getParam<bool>("set_start_inactive"))
    {
      for (const auto & inactive_elem : meshBase().element_ptr_range())
        if (!inactive_elem->active())
          ray->setStart(inactive_elem->true_centroid(), inactive_elem);
    }

    ray->setStart(elem->vertex_average(), elem);

    if (getParam<bool>("set_start_again"))
      ray->setStart(1.01 * elem->vertex_average());
    if (getParam<bool>("set_direction_again"))
    {
      ray->setStartingDirection(Point(1, 0, 0));
      ray->setStartingDirection(Point(-1, 0, 0));
    }
    if (getParam<bool>("set_zero_direction"))
      ray->setStartingDirection(Point(0, 0, 0));
    if (getParam<bool>("set_end_equal_start"))
      ray->setStartingEndPoint(ray->currentPoint());
    if (getParam<bool>("set_end_with_direction"))
    {
      ray->setStartingDirection(Point(1, 0, 0));
      ray->setStartingEndPoint(elem->point(0));
    }
    if (getParam<bool>("set_distance_with_end"))
    {
      ray->setStartingEndPoint(elem->point(0));
      ray->setStartingMaxDistance(1);
    }
    if (getParam<bool>("set_end_with_distance"))
    {
      ray->setStartingMaxDistance(1);
      ray->setStartingEndPoint(elem->point(0));
    }
    if (getParam<bool>("set_end_fail_bbox"))
      ray->setStartingEndPoint(Point(1e6, 1e6, 1e6));
    if (getParam<bool>("set_distance_negative"))
      ray->setStartingMaxDistance(-1);
  }
}
