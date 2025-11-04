//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary1PhaseBase.h"

/**
 * Base class for boundary components connected to FlowChannel1Phase components
 */
class FlowBoundary1Phase : public FlowBoundary1PhaseBase
{
public:
  FlowBoundary1Phase(const InputParameters & params);

protected:
  virtual void check() const override;

  /**
   * Creates the boundary condition objects for 1-phase flow
   */
  virtual void addWeakBCs();

public:
  static InputParameters validParams();
};
