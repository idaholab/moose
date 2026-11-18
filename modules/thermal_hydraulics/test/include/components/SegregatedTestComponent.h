//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component1D.h"

/**
 * Test component that solves two Poisson equations, each in its own solver system,
 * coupled through a fixed-point iteration.
 *
 * The equations are
 *   -div(grad(u)) = 0,         u(in) = 0, u(out) = 1
 *   -div(grad(v)) = source,    v(in) = 0, v(out) = 0
 * where 'u' and 'v' are added to the solver systems named by the 'u_solver_system'
 * and 'v_solver_system' parameters.
 */
class SegregatedTestComponent : public Component1D
{
public:
  SegregatedTestComponent(const InputParameters & parameters);

protected:
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /// Name of the driving variable
  const VariableName _u_var_name;
  /// Name of the variable coupled to the driving variable
  const VariableName _v_var_name;
  /// Solver system to which the driving variable is added
  const SolverSystemName _u_solver_system;
  /// Solver system to which the coupled variable is added
  const SolverSystemName _v_solver_system;

public:
  static InputParameters validParams();
};
