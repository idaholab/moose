//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRayBC.h"

/**
 * A RayBC that generates an additional Ray at the intersection point
 * on the boundary in the opposite direction of the Ray being worked on.
 *
 * Rays created mid-trace cannot create additional rays.
 */
class CreateRayRayBCTest : public GeneralRayBC
{
public:
  CreateRayRayBCTest(const InputParameters & params);

  static InputParameters validParams();

  virtual void onBoundary(const unsigned int num_applying) override;

protected:
  /// Ray's aux data index for a value that states if a Ray is a secondary ray (generated-mid trace)
  const RayDataIndex _secondary_ray_data_index;
};
