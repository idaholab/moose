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
 * Boundary condition with prescribed stagnation pressure and temperature for 1-phase flow channels
 */
class PhysicsInletStagnationPressureTemperature : public PhysicsFlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsInletStagnationPressureTemperature(const InputParameters & params);

  virtual bool isReversible() const override { return _reversible; }

protected:
  virtual void init() override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;
};
