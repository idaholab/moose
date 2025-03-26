//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"
#include "PhysicsComponentInterface.h"

/**
 * Test class for the PhysicsComponentInterface errors
 */
class TestPhysicsComponentInterfaceErrors : virtual public PhysicsBase,
                                            virtual public PhysicsComponentInterface
{
public:
  static InputParameters validParams();

  TestPhysicsComponentInterfaceErrors(const InputParameters & parameters);
};
