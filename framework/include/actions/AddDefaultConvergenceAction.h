//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Adds default Convergence objects to the simulation.
 */
class AddDefaultConvergenceAction : public Action
{
public:
  static InputParameters validParams();

  AddDefaultConvergenceAction(const InputParameters & params);

  virtual void act() override;

protected:
  /// Adds the default nonlinear Convergence object(s)
  void addDefaultNonlinearConvergence();
  /// Adds the default fixed point Convergence object
  void addDefaultFixedPointConvergence();

  /**
   * Checks that nonlinear convergence parameters were not set in the executioner
   * if using a Convergence object that does not use them.
   */
  void checkUnusedNonlinearConvergenceParameters();
  /**
   * Checks that fixed point convergence parameters were not set in the executioner
   * if using a Convergence object that does not use them.
   */
  void checkUnusedFixedPointConvergenceParameters();
};
