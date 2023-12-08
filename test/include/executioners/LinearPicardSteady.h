//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"
#include "libmesh/solver_configuration.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Solver configuration class used with the linear solvers in a SIMPLE solver.
 */
class LinearPicardSolverConfiguration : public libMesh::SolverConfiguration
{
  /**
   * Override this to make sure the PETSc options are not overwritten in the linear solver
   */
  virtual void configure_solver() override {}
};

/**
 * LinearPicardSteady executioners call "solve()" on two different nonlinear systems in sequence
 */
class LinearPicardSteady : public Executioner
{
public:
  static InputParameters validParams();

  LinearPicardSteady(const InputParameters & parameters);

  void init() override;
  void execute() override;
  bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  void originalSolve();
  void newSolve();

  FEProblemBase & _problem;

  Real _system_time;
  int & _time_step;
  Real & _time;

private:
  const unsigned int _linear_sys_number;
  bool _last_solve_converged;
  unsigned int _number_of_iterations;
};
