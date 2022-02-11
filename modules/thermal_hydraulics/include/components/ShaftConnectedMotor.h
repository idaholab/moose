//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"
#include "ShaftConnectable.h"

/**
 * Motor to drive a shaft component
 */
class ShaftConnectedMotor : public Component, public ShaftConnectable
{
public:
  ShaftConnectedMotor(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Torque
  const Real & _torque;
  /// Moment of intertia
  const Real & _inertia;

public:
  static InputParameters validParams();
};
