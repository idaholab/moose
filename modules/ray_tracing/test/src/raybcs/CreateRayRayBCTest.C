//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateRayRayBCTest.h"

// Local includes
#include "RayTracingStudy.h"

registerMooseObject("RayTracingTestApp", CreateRayRayBCTest);

InputParameters
CreateRayRayBCTest::validParams()
{
  return GeneralRayBC::validParams();
}

CreateRayRayBCTest::CreateRayRayBCTest(const InputParameters & params)
  : GeneralRayBC(params),
    _secondary_ray_data_index(_study.registerRayAuxData(name() + "_secondary"))
{
}

void
CreateRayRayBCTest::onBoundary(const unsigned int /* num_applying */)
{
  // Don't generate another if this Ray is already a secondary Ray (was generated mid-trace)
  const bool is_secondary = currentRay()->auxData(_secondary_ray_data_index) > 0;
  if (is_secondary)
    return;

  // Reverse direction of the Ray - where the new Ray will go
  const Point reverse_direction = -1.0 * currentRay()->direction();

  // Get a new Ray that starts in this elem, at the current intersection point in the
  // reverse direction, from the current interescted side, with a unique ID
  std::shared_ptr<Ray> new_ray = acquireRay(reverse_direction);
  // Set that this Ray is a secondary Ray so that it doesn't generate even more Rays
  new_ray->auxData(_secondary_ray_data_index) = 1;
  // Add it to the buffer to be traced and we're done!
  moveRayToBuffer(new_ray);
}
