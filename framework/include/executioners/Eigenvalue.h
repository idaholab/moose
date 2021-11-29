//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#include "Executioner.h"

class InputParameters;
class EigenProblem;

template <typename T>
InputParameters validParams();

/**
 * Eigenvalue executioner is used to drive the eigenvalue calculations. At the end,
 * SLEPc will be involved.
 * We derive from Executioner instead of Steady because 1) we want to have a fine-grain
 * control such as recovering; 2) Conceptually, Steady is very different from Eigenvalue,
 * where the former handles a nonlinear system of equations while the later targets
 * at an eigenvalue problem.
 */
class Eigenvalue : public Executioner
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  static InputParameters validParams();

  Eigenvalue(const InputParameters & parameters);

  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

#ifdef LIBMESH_HAVE_SLEPC
  virtual void init() override;

  /**
   * Eigenvalue executioner does not allow time kernels
   */
  virtual void checkIntegrity();

  /**
   * Get the number of grid sequencing steps
   */
  unsigned int numGridSteps() const { return _feproblem_solve.numGridSteps(); }

private:
  /**
   * Prepare right petsc options
   */
  void prepareSolverOptions();
#endif

protected:
  EigenProblem & _eigen_problem;

  /// inner-most solve object to perform Newton solve with SLEPc
  FEProblemSolve _feproblem_solve;

  /// Postprocessor value that scales solution when eigensolve is finished
  const PostprocessorValue * const _normalization;

  Real _system_time;
  int & _time_step;
  Real & _time;

  PerfID _final_timer;

private:
  bool _last_solve_converged;
};
