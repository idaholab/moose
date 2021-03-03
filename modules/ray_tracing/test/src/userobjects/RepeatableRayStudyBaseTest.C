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

  params.suppressParameter<std::vector<Real>>("max_distances");
  params.suppressParameter<std::vector<Point>>("end_points");

  return params;
}

RepeatableRayStudyBaseTest::RepeatableRayStudyBaseTest(const InputParameters & parameters)
  : RepeatableRayStudy(parameters)
{
}

void
RepeatableRayStudyBaseTest::defineRays()
{
  if (getParam<bool>("define_no_rays"))
    return;

  RepeatableRayStudy::defineRays();

  if (parameters().isParamSetByUser("create_non_unique_id_rays") &&
      getParam<bool>("create_non_unique_id_rays") && rays().size())
    defineRay(acquireCopiedRay(*rays()[0]));

  if (parameters().isParamSetByUser("create_additional_ray_pid"))
  {
    const auto pid = getParam<processor_id_type>("create_additional_ray_pid");

    mooseAssert(pid < n_processors(), "Invalid pid");
    auto ray = acquireRegisteredRay("additional_ray");

    if (pid == _pid && rays().size())
    {
      ray->setStart(rays()[0]->currentPoint());
      ray->setStartingDirection(rays()[0]->direction());
      defineRay(std::move(ray));
    }
  }

  if (parameters().isParamSetByUser("create_non_replicated_id_ray_pid"))
  {
    const auto pid = getParam<processor_id_type>("create_non_replicated_id_ray_pid");

    mooseAssert(pid < n_processors(), "Invalid pid");
    auto ray = acquireRegisteredRay("non_replicated_ray");

    if (pid == _pid && rays().size())
    {
      ray->setStart(rays()[0]->currentPoint());
      ray->setStartingDirection(rays()[0]->direction());

      auto & non_const_rays =
          *const_cast<std::vector<MooseUtils::SharedPool<Ray>::PtrType> *>(&rays());
      non_const_rays[0] = std::move(ray);
    }
  }
}

void
RepeatableRayStudyBaseTest::modifyRay(Ray & ray)
{
  if (parameters().isParamSetByUser("create_non_replicated_ray_pid") && ray.id() == 0)
  {
    const auto pid = getParam<processor_id_type>("create_non_replicated_ray_pid");

    mooseAssert(pid < n_processors(), "Invalid pid");

    if (pid == _pid)
      ray.setStartingMaxDistance(1);
  }
}
