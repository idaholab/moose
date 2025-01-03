//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsVolumeJunction.h"

/**
 * Pump between 1-phase flow channels that has a non-zero volume
 */
class PhysicsPump : public PhysicsVolumeJunction
{
public:
  static InputParameters validParams();

  PhysicsPump(const InputParameters & params);

protected:
  virtual void init() override;

  /// Pump head [m]
  const Real & _head;
};
