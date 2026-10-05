//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Test component that exposes a controllable boolean parameter
 *
 * The boolean value is forwarded to a backing user object and connected as a
 * controllable component parameter, so that the component boolean control and
 * postprocessor objects can be exercised independently of any physics.
 */
class BooleanValueTestComponent : public Component
{
public:
  BooleanValueTestComponent(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// Controllable boolean value forwarded to the backing user object
  const bool & _value;

public:
  static InputParameters validParams();
};
