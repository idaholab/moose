//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/system.h"

class NonlinearSystemBase;
class SystemBase;

/**
 * The libMesh constraint object of a system. It adds the exact constraints of every active
 * Constraint of a nonlinear system each time libMesh rebuilds the constraints. A displaced problem
 * gets a second hub for the displaced copy of the system.
 */
class MultiPointConstraintHub : public libMesh::System::Constraint
{
public:
  /**
   * @param sys The nonlinear system holding the constraints to apply
   * @param constrained_system \p sys or its displaced copy, whose DofMap receives the constraints
   */
  MultiPointConstraintHub(NonlinearSystemBase & sys, SystemBase & constrained_system);

  virtual void constrain() override;

  /**
   * Rebuild the constraints of the constrained system. Only coefficients may change: the sparsity
   * pattern is not rebuilt.
   */
  void reinit();

private:
  /// The nonlinear system whose constraints this hub applies
  NonlinearSystemBase & _sys;

  /// The system whose DofMap receives the exact constraints
  SystemBase & _constrained_sys;
};
