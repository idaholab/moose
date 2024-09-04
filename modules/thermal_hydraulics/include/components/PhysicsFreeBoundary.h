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
 * Adds the boundary terms resulting from an integration by parts of the
 * advection terms, using no external boundary data.
 */
class PhysicsFreeBoundary : public PhysicsFlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsFreeBoundary(const InputParameters & parameters);

  virtual void addMooseObjects() override;
};
