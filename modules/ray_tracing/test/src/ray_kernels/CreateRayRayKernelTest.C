//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateRayRayKernelTest.h"

// Local includes
#include "RayTracingStudy.h"

registerMooseObject("RayTracingTestApp", CreateRayRayKernelTest);

InputParameters
CreateRayRayKernelTest::validParams()
{
  auto params = GeneralRayKernel::validParams();
  return params;
}

CreateRayRayKernelTest::CreateRayRayKernelTest(const InputParameters & params)
  : GeneralRayKernel(params),
    _secondary_ray_data_index(_study.registerRayAuxData(name() + "_secondary"))
{
}

void
CreateRayRayKernelTest::onSegment()
{
  // Don't generate another if this Ray is already a secondary Ray (was generated mid-trace)
  const bool is_secondary = currentRay()->auxData(_secondary_ray_data_index) > 0;
  if (is_secondary)
    return;

  // Midpoint of the Ray segment - where the new Ray will start
  const Point midpoint = 0.5 * (_current_segment_start + _current_segment_end);
  // Reverse direction of the Ray - where the new Ray will go
  const Point reverse_direction = -1.0 * currentRay()->direction();

  // Get a new Ray that starts in this elem, at the midpoint in the
  // reverse direction, with a unique ID
  std::shared_ptr<Ray> new_ray = acquireRay(midpoint, reverse_direction);
  // Set that this Ray is a secondary Ray so that it doesn't generate even more Rays
  new_ray->auxData(_secondary_ray_data_index) = 1;
  // Add it to the buffer to be traced and we're done!
  moveRayToBuffer(new_ray);
}
