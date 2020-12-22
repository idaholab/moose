//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRayKernel.h"

// Forward declarations
class TestRayDataStudy;

/**
 * RayKernel to be used in conjunction with TestRayDataStudy
 */
class TestRayDataRayKernel : public GeneralRayKernel
{
public:
  TestRayDataRayKernel(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override;

protected:
  const TestRayDataStudy & _test_ray_data_study;
};
