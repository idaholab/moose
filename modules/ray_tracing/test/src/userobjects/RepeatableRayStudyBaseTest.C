//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RepeatableRayStudyBaseTest.h"

registerMooseObject("RayTracingTestApp", RepeatableRayStudyBaseTest);
registerMooseObject("RayTracingTestApp", RepeatableRayStudyDefineNoClaimTest);

InputParameters
RepeatableRayStudyBaseTest::validParams()
{
  auto params = RepeatableRayStudy::validParams();

  params.addParam<bool>("create_non_unique_id_rays",
                        "Create Rays on each processor with a non-unique ID");

  params.addParam<processor_id_type>("create_additional_ray_pid",
                                     "Create an additional Ray on the given processor");
  params.addParam<processor_id_type>(
      "create_non_replicated_id_ray_pid",
      "Create a Ray with a non-replicated ID on the given processor");
  params.addParam<processor_id_type>("create_non_replicated_ray_pid",
                                     "Create a non-replicated Ray on the given processor");

  params.addParam<bool>("define_no_rays", false, "Test defining no rays");
  params.addParam<bool>("define_nullptr_ray", false, "Test defining a nullptr ray");
  params.addParam<bool>(
      "define_ray_with_starting_elem", false, "Test defining a ray with a starting element");

  params.suppressParameter<std::vector<Real>>("max_distances");
  params.suppressParameter<std::vector<Point>>("end_points");

  return params;
}

InputParameters
RepeatableRayStudyDefineNoClaimTest::validParams()
{
  auto params = RepeatableRayStudy::validParams();
  params.set<bool>("_claim_after_define_rays") = false;
  return params;
}

RepeatableRayStudyBaseTest::RepeatableRayStudyBaseTest(const InputParameters & parameters)
  : RepeatableRayStudy(parameters)
{
}

RepeatableRayStudyDefineNoClaimTest::RepeatableRayStudyDefineNoClaimTest(
    const InputParameters & parameters)
  : RepeatableRayStudy(parameters)
{
}

void
RepeatableRayStudyBaseTest::defineRays()
{
  if (getParam<bool>("define_no_rays"))
    return;

  RepeatableRayStudy::defineRays();

  if (getParam<bool>("define_nullptr_ray") && _rays.size())
    _rays[0] = nullptr;

  if (parameters().isParamSetByUser("create_non_unique_id_rays") &&
      getParam<bool>("create_non_unique_id_rays"))
    _rays.push_back(acquireCopiedRay(*_rays[0]));

  if (parameters().isParamSetByUser("create_additional_ray_pid"))
  {
    const auto pid = getParam<processor_id_type>("create_additional_ray_pid");

    mooseAssert(pid < n_processors(), "Invalid pid");
    auto ray = acquireRegisteredRay("additional_ray");

    if (pid == _pid)
    {
      ray->setStart(_rays[0]->currentPoint());
      ray->setStartingDirection(_rays[0]->direction());
      _rays.push_back(ray);
    }
  }

  if (parameters().isParamSetByUser("create_non_replicated_id_ray_pid"))
  {
    const auto pid = getParam<processor_id_type>("create_non_replicated_id_ray_pid");

    mooseAssert(pid < n_processors(), "Invalid pid");
    auto ray = acquireRegisteredRay("non_replicated_ray");

    if (pid == _pid)
    {
      ray->setStart(_rays[0]->currentPoint());
      ray->setStartingDirection(_rays[0]->direction());
      _rays[0] = ray;
    }
  }

  if (parameters().isParamSetByUser("create_non_replicated_ray_pid"))
  {
    const auto pid = getParam<processor_id_type>("create_non_replicated_ray_pid");

    mooseAssert(pid < n_processors(), "Invalid pid");

    if (pid == _pid)
      _rays[0]->setStartingMaxDistance(1);
  }

  if (getParam<bool>("define_ray_with_starting_elem"))
  {
    auto ray = acquireRegisteredRay("starting_elem_ray");
    ray->setStart(Point(0, 0, 0), _mesh.queryElemPtr(0));
    ray->setStartingDirection(Point(1, 0, 0));
    _rays.push_back(ray);
  }
}
