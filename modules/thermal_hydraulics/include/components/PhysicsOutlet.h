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
 * Boundary condition with prescribed pressure for flow channels using Physics
 */
class PhysicsOutlet : public PhysicsFlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsOutlet(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;
};
