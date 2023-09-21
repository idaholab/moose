//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestTransientRaysStudy.h"

#include "Function.h"

registerMooseObject("RayTracingTestApp", TestTransientRaysStudy);

InputParameters
TestTransientRaysStudy::validParams()
{
  auto params = RayTracingStudy::validParams();
  params.addRequiredParam<FunctionName>("distance_function",
                                        "The function to sample for the ray distances");
  params.addRequiredParam<BoundaryName>("boundary", "The boundary to spawn rays from");
  params.set<bool>("_use_ray_registration") = false;
  return params;
}

TestTransientRaysStudy::TestTransientRaysStudy(const InputParameters & parameters)
  : RayTracingStudy(parameters),
    _distance_function(getFunction("distance_function")),
    _boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("boundary"))),
    _generated_rays(declareRestartableData<bool>("generated_rays", false)),
    _banked_rays(
        declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("banked_rays", this))
{
}

void
TestTransientRaysStudy::generateRays()
{
  if (!_generated_rays)
  {
    for (const auto bnd_elem : *_mesh.getBoundaryElementRange())
      if (bnd_elem->_elem->processor_id() == processor_id() && bnd_elem->_bnd_id == _boundary_id)
      {
        const auto elem = bnd_elem->_elem;
        const auto side = elem->build_side_ptr(bnd_elem->_side);
        const auto start_point = side->vertex_average();

        auto ray = acquireRay();
        ray->setStart(start_point, elem, bnd_elem->_side);
        ray->setStartingDirection(elem->vertex_average() - start_point);
        ray->setStartingMaxDistance(_distance_function.value(_fe_problem.time(), start_point));
        moveRayToBuffer(ray);
      }

    _generated_rays = true;
  }
  else
  {
    for (auto & ray : _banked_rays)
    {
      const auto start_point = ray->currentPoint();
      const auto direction = ray->direction();
      const auto elem = ray->currentElem();

      ray->resetCounters();
      ray->clearStartingInfo();
      ray->setStart(start_point, elem);
      ray->setStartingDirection(direction);
      ray->setStartingMaxDistance(_distance_function.value(_fe_problem.time(), start_point));
    }
    moveRaysToBuffer(_banked_rays);
    _banked_rays.clear();
  }
}

void
TestTransientRaysStudy::postExecuteStudy()
{
  _banked_rays = rayBank();
}
