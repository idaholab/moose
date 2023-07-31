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

// System includes
#include <string>

// Forward declarations
class InputParameters;
class FEProblemBase;

template <typename T>
InputParameters validParams();

/**
 * Steady executioners usually only call "solve()" on the NonlinearSystem once.
 */
class Steady : public Executioner
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  static InputParameters validParams();

  Steady(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

  virtual void checkIntegrity();

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

  /**
   * Get a general iteration number for the purpose of outputting, useful in the presence of a
   * nested solve This output iteration number may be set by the parent app for a sub-app. This
   * behavior is decided by the Executioner/Executor/SolveObject in charge of the solve.
   */
  virtual unsigned int getIterationNumberOutput() const { return _output_iteration_number; }

  /**
   * Set a general iteration number for the purpose of outputting, useful in the presence of a
   * nested solve This output iteration number may be set by the parent app for a sub-app, e.g.
   * OptimizeSolve.
   */
  virtual void setIterationNumberOutput(unsigned int iteration_number)
  {
    _output_iteration_number = iteration_number;
  }

protected:
  FEProblemBase & _problem;

  FEProblemSolve _feproblem_solve;

  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Iteration number obtained from the main application
  unsigned int _output_iteration_number;

private:
  bool _last_solve_converged;
};
