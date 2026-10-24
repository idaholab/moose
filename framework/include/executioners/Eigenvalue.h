//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#include "Executioner.h"
#include "EigenProblemSolve.h"
#include "MooseEnum.h"

class InputParameters;
class EigenProblem;

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
  static InputParameters validParams();

  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
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
  unsigned int numGridSteps() const { return _eigen_problem_solve.numGridSteps(); }
#endif

protected:
  EigenProblem & _eigen_problem;

  /// inner-most solve object to perform Newton solve with SLEPc
  EigenProblemSolve _eigen_problem_solve;

  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Whether one output step is written per converged eigenvector instead of the active one only
  const bool _output_all_eigenvectors;

  /// What the output time of each eigenvector step is
  const MooseEnum _eigenvector_time;

  PerfID _final_timer;

private:
#ifdef LIBMESH_HAVE_SLEPC
  /**
   * Copy the converged eigenvector \p i into the solution of the eigen system, refresh the objects
   * that execute on linear iterations, scale the eigenvector with the normalization hook, and
   * execute the objects that execute at the end of a time step.
   */
  void loadEigenvector(dof_id_type i);

  /**
   * @return The output time of the step holding the converged eigenvector \p i, whose eigenvalue
   * is \p eig as (real, imaginary), according to the 'eigenvector_time' parameter
   */
  Real eigenvectorTime(dof_id_type i, const std::pair<Real, Real> & eig) const;
#endif

  bool _last_solve_converged = true;
};
