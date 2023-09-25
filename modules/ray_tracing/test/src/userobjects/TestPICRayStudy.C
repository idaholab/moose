//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPICRayStudy.h"

#include "ClaimRays.h"
#include "Function.h"

registerMooseObject("RayTracingTestApp", TestPICRayStudy);

InputParameters
TestPICRayStudy::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addRequiredParam<std::vector<Point>>("start_points",
                                              "The point(s) where the ray(s) start");
  params.addRequiredParam<std::vector<Point>>(
      "start_directions",
      "The direction(s) that the ray(s) start in (does not need to be normalized)");
  params.addRequiredParam<FunctionName>(
      "velocity_function", "A function that describes the velocity field for the ray(s)");

  // We're not going to use registration because we don't care to name our rays because
  // we will have a lot of them
  params.set<bool>("_use_ray_registration") = false;

  return params;
}

TestPICRayStudy::TestPICRayStudy(const InputParameters & parameters)
  : RayTracingStudy(parameters),
    _start_points(getParam<std::vector<Point>>("start_points")),
    _start_directions(getParam<std::vector<Point>>("start_directions")),
    _velocity_function(getFunction("velocity_function")),
    _has_generated(declareRestartableData<bool>("has_generated", false)),
    _banked_rays(
        declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("_banked_rays", this))
{
  if (_start_points.size() != _start_directions.size())
    paramError("start_directions", "Must be the same size as 'start_points'");
}

void
TestPICRayStudy::generateRays()
{
  // We generate rays the first time only, after that we will
  // pull from the bank and update velocities/max distances
  if (!_has_generated)
  {
    // The unclaimed rays that we're going to generate
    // Here we need to "claim" rays because in parallel, we have
    // a list of points but do not know which processor will
    // own the point that that ray starts in. So, we duplicate
    // the rays on all processors and then let one processor pick them.
    // Basically - we fill them here and then pass them to a ClaimRays
    // object to do all of the magic. In a real PIC case, we'll just
    // generate the rays for the local rays that we care about
    // and the claiming probably won't be necessary
    std::vector<std::shared_ptr<Ray>> rays(_start_points.size());

    // Create a ray for each point/direction/velocity triplet
    // Note that instead of keeping track of the velocity, we're
    // just going to set the maximum distance that a ray can
    // travel based on the timestep * the starting velocity.
    for (const auto i : index_range(_start_points))
    {
      rays[i] = acquireReplicatedRay();
      rays[i]->setStart(_start_points[i]);
      rays[i]->setStartingDirection(_start_directions[i].unit());
      rays[i]->setStartingMaxDistance(maxDistance(*rays[i]));
    }

    // Claim the rays
    std::vector<std::shared_ptr<Ray>> claimed_rays;
    ClaimRays claim_rays(*this, rays, claimed_rays, false);
    claim_rays.claim();
    // ...and then add them to be traced
    moveRaysToBuffer(claimed_rays);
  }
  // Rays are in the bank: reset them and update the velocities / end distance
  else
  {
    // Reset each ray
    for (auto & ray : _banked_rays)
    {
      // Store off the ray's info before we reset it
      const auto start_point = ray->currentPoint();
      const auto direction = ray->direction();
      const auto elem = ray->currentElem();

      // Reset it (this is required to reuse a ray)
      ray->resetCounters();
      ray->clearStartingInfo();

      // And set the new starting information
      ray->setStart(start_point, elem);
      ray->setStartingDirection(direction);
      ray->setStartingMaxDistance(maxDistance(*ray));
    }
    // Add the rays to be traced
    moveRaysToBuffer(_banked_rays);
    _banked_rays.clear();
  }

  _has_generated = true;
}

void
TestPICRayStudy::postExecuteStudy()
{
  // Copy the rays that are banked in the study into our own bank
  _banked_rays = rayBank();
}

Real
TestPICRayStudy::maxDistance(const Ray & ray) const
{
  // velocity * dt
  return _velocity_function.value(_fe_problem.time(), ray.currentPoint()) * _fe_problem.dt();
}
