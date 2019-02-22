//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEPROBLEMSOLVE_H
#define FEPROBLEMSOLVE_H

#include "SolveObject.h"

class FEProblemSolve;

template <>
InputParameters validParams<FEProblemSolve>();

/// Enumeration for nonlinear convergence reasons
enum class MooseNonlinearConvergenceReason
{
  ITERATING = 0,
  CONVERGED_FNORM_ABS = 2,
  CONVERGED_FNORM_RELATIVE = 3,
  CONVERGED_SNORM_RELATIVE = 4,
  DIVERGED_FUNCTION_COUNT = -2,
  DIVERGED_FNORM_NAN = -4,
  DIVERGED_LINE_SEARCH = -6
};

// The idea with these enums is to abstract the reasons for
// convergence/divergence, i.e. they could be used with linear algebra
// packages other than PETSc.  They were directly inspired by PETSc,
// though.  This enum could also be combined with the
// MooseNonlinearConvergenceReason enum but there might be some
// confusion (?)
enum class MooseLinearConvergenceReason
{
  ITERATING = 0,
  // CONVERGED_RTOL_NORMAL        =  1,
  // CONVERGED_ATOL_NORMAL        =  9,
  CONVERGED_RTOL = 2,
  CONVERGED_ATOL = 3,
  CONVERGED_ITS = 4,
  // CONVERGED_CG_NEG_CURVE       =  5,
  // CONVERGED_CG_CONSTRAINED     =  6,
  // CONVERGED_STEP_LENGTH        =  7,
  // CONVERGED_HAPPY_BREAKDOWN    =  8,
  DIVERGED_NULL = -2,
  // DIVERGED_ITS                 = -3,
  // DIVERGED_DTOL                = -4,
  // DIVERGED_BREAKDOWN           = -5,
  // DIVERGED_BREAKDOWN_BICG      = -6,
  // DIVERGED_NONSYMMETRIC        = -7,
  // DIVERGED_INDEFINITE_PC       = -8,
  DIVERGED_NANORINF = -9,
  // DIVERGED_INDEFINITE_MAT      = -10
  DIVERGED_PCSETUP_FAILED = -11
};

class FEProblemSolve : public SolveObject
{
public:
  FEProblemSolve(Executioner * ex);

  /**
   * Picard solve the FEProblem.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  /**
   * Check to see if an exception has occurred on any processor and stop the solve.
   *
   * Note: Collective on MPI!  Must be called simultaneously by all processors!
   *
   * Also: This will throw a MooseException!
   *
   * Note: DO NOT CALL THIS IN A THREADED REGION!  This is meant to be called just after a threaded
   * section.
   */
  virtual void checkExceptionAndStopSolve();

  virtual Real finalNonlinearResidual() const;
  virtual unsigned int nNonlinearIterations() const;
  virtual unsigned int nLinearIterations() const;

  /**
   * Check for converence of the nonlinear solution
   * @param msg            Error message that gets sent back to the solver
   * @param it             Iteration counter
   * @param xnorm          Norm of the solution vector
   * @param snorm          Norm of the change in the solution vector
   * @param fnorm          Norm of the residual vector
   * @param rtol           Relative residual convergence tolerance
   * @param stol           Solution change convergence tolerance
   * @param abstol         Absolute residual convergence tolerance
   * @param nfuncs         Number of function evaluations
   * @param max_funcs      Maximum Number of function evaluations
   * @param initial_residual_before_preset_bcs      Residual norm prior to imposition of PresetBC
   * values on solution vector
   * @param div_threshold  Maximum value of residual before triggering divergence check
   */
  virtual MooseNonlinearConvergenceReason
  checkNonlinearConvergence(std::string & msg,
                            const PetscInt it,
                            const Real xnorm,
                            const Real snorm,
                            const Real fnorm,
                            const Real rtol,
                            const Real stol,
                            const Real abstol,
                            const PetscInt nfuncs,
                            const PetscInt max_funcs,
                            const PetscBool force_iteration,
                            const Real initial_residual_before_preset_bcs,
                            const Real div_threshold);

  /**
   * Check for convergence of the linear solution
   * @param msg            Error message that gets sent back to the solver
   * @param n              Iteration counter
   * @param rnorm          Norm of the residual vector
   * @param rtol           Relative residual convergence tolerance
   * @param atol           Absolute residual convergence tolerance
   * @param dtol           Divergence tolerance
   * @param maxits         Maximum number of linear iterations allowed
   */
  virtual MooseLinearConvergenceReason checkLinearConvergence(std::string & msg,
                                                              const PetscInt n,
                                                              const Real rnorm,
                                                              const Real rtol,
                                                              const Real atol,
                                                              const Real dtol,
                                                              const PetscInt maxits);

  virtual void setInnerSolve(SolveObject &) override
  {
    mooseError("Cannot set inner solve for FEProblemSolve");
  }

protected:
  /// Splitting
  std::vector<std::string> _splitting;
  /// Flag to completely bypass solve
  bool _no_feproblem_solve;

private:
  /// Timer for Picard iteration
  const PerfID _solve_timer;
  const PerfID _check_nonlinear_convergence_timer;
  const PerfID _check_linear_convergence_timer;
  const PerfID _check_exception_and_stop_solve_timer;

  bool _fail_next_linear_convergence_check;
};
#endif // FEPROBLEMSOLVE_H
