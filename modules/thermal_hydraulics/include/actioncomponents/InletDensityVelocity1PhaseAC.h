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
 * Boundary condition with prescribed density and velocity for a FlowChannel1PhaseAC.
 */
class InletDensityVelocity1PhaseAC : public FlowBoundary1PhaseAC
{
public:
  static InputParameters validParams();
  InletDensityVelocity1PhaseAC(const InputParameters & params);

protected:
  virtual void addUserObjects() override;

  /// Prescribed density
  const Real _rho;
  /// Prescribed velocity
  const Real _vel;
  /// True for reversible, false for pure inlet
  const bool _reversible;
};
