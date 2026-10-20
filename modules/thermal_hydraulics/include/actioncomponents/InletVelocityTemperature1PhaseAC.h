//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary1PhaseAC.h"

/**
 * Boundary condition with prescribed velocity and temperature for a FlowChannel1PhaseAC.
 */
class InletVelocityTemperature1PhaseAC : public FlowBoundary1PhaseAC
{
public:
  static InputParameters validParams();
  InletVelocityTemperature1PhaseAC(const InputParameters & params);

protected:
  virtual void addUserObjects() override;

  /// Prescribed velocity
  const Real _vel;
  /// Prescribed temperature
  const Real _T;
  /// True for reversible, false for pure inlet
  const bool _reversible;
};
