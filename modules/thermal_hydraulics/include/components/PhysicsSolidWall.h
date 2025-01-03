//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsFlowBoundary.h"

/**
 * Component for solid wall BC for 1D flow implemented using Physics
 */
class PhysicsSolidWall : public PhysicsFlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsSolidWall(const InputParameters & params);

  virtual void init() override;
};
