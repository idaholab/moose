//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * A RayKernel that does nothing.
 */
class NullRayKernel : public GeneralRayKernel
{
public:
  NullRayKernel(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override;
};
