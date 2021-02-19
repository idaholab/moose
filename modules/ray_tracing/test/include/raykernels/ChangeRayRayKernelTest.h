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

class ChangeRayRayKernelTest : public GeneralRayKernel
{
public:
  ChangeRayRayKernelTest(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override;

protected:
  const RayDataIndex _ray_data_index;
  const Real _add_value;
  const Real _scale_value;
};
