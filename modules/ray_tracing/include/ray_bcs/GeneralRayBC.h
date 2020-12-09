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
#include "RayBoundaryConditionBase.h"

class GeneralRayBC : public RayBoundaryConditionBase
{
public:
  GeneralRayBC(const InputParameters & params);

  static InputParameters validParams();

  virtual void onBoundary(const unsigned int num_applying) = 0;
};
