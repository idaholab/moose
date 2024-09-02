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
 * Boundary condition with prescribed mass flow rate and temperature for flow channels
 * leveraging the Physics system
 */
class PhysicsInletMassFlowRateTemperature : public PhysicsFlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsInletMassFlowRateTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

  virtual bool isReversible() const override { return _reversible; }

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;
};
