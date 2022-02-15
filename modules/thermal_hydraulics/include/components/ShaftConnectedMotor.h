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
  virtual void check() const override;

  /// Torque function name
  const FunctionName & _torque_fn_name;
  /// Moment of inertia function name
  const FunctionName & _inertia_fn_name;

public:
  static InputParameters validParams();
};
