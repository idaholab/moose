//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowJunction.h"

/**
 * Base class for flow junctions between components using Physics for the flow equations
 * discretization
 */
class PhysicsFlowJunction : public FlowJunction
{
public:
  static InputParameters validParams();

  PhysicsFlowJunction(const InputParameters & params);

protected:
  virtual void init() override;
  virtual void check() const override;

  /// Thermal hydraulics Physics active on this flow junction
  std::set<ThermalHydraulicsFlowPhysics *> _th_physics;
};
