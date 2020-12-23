//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local Includes
#include "GeneralRayKernel.h"

/**
 * A RayKernel that generates an additional Ray at the midpoint of each
 * segment in the opposite direction of the Ray being worked on.
 *
 * Rays created mid-trace cannot create additional rays.
 */
class CreateRayRayKernelTest : public GeneralRayKernel
{
public:
  CreateRayRayKernelTest(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override;

protected:
  /// Ray's aux data index for a value that states if a Ray is a secondary ray (generated-mid trace)
  const RayDataIndex _secondary_ray_data_index;
};
