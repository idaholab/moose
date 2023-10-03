//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsDiscretization.h"

/**
 * Use a continuous Galerkin (continuous finite element) for this Physics
 */
class ContinuousGalerkin : public PhysicsDiscretization
{
public:
  static InputParameters validParams();

  ContinuousGalerkin(const InputParameters & parameters);
};
