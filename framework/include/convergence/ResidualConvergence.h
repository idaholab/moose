//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"
#include "PerfGraphInterface.h"

// PETSc includes
#include <petsc.h>
#include <petscmat.h>

/**
 * The ResidualConvergence is designed to provide a more flexible convergence
 * criteria and user access to each iteration computation. It allows for the user to
 * specify the relative and absolute tolerance for the residual, as well as
 * the maximum number of iterations and function evaluations. The ResidualConvergence
 * class also allows for the user to specify the convergence tolerance for the
 * linear solver.
 */
class ResidualConvergence : public Convergence
{
public:
  static InputParameters validParams();

  static InputParameters residualConvergenceParams();

  ResidualConvergence(const InputParameters & parameters);

  Convergence::MooseConvergenceStatus checkConvergence() override;

protected:
  /**
   * Check the relative convergence of the nonlinear solution
   * @param fnorm          Norm of the residual vector
   * @param the_residual   The residual to check
   * @param rtol           Relative tolerance
   * @param abstol         Absolute tolerance
   * @return               Bool signifying convergence
   */
  virtual bool checkRelativeConvergence(const PetscInt it,
                                        const Real fnorm,
                                        const Real the_residual,
                                        const Real rtol,
                                        const Real abstol,
                                        std::ostringstream & oss);

  /**
   * Performs setup necessary for each call to checkConvergence
   */
  virtual void nonlinearConvergenceSetup(){};

  FEProblemBase & _fe_problem;

  // Variables for the convergence criteria
  Real _atol; // absolute convergence tolerance
  Real _rtol; // relative convergence tolerance
  Real _stol; // convergence (step) tolerance in terms of the norm of the change in the
              // solution between steps

  Real _div_threshold = std::numeric_limits<Real>::max();
  /// the absolute non linear divergence tolerance
  Real _nl_abs_div_tol = -1;
  Real _divtol; // relative divergence tolerance

  Real _nl_rel_tol;
  Real _nl_abs_tol;
  Real _nl_rel_step_tol;
  Real _nl_abs_step_tol;

  int _nl_forced_its = 0; // the number of forced nonlinear iterations
  PetscInt _nfuncs = 0;

  unsigned int _nl_max_its;
  unsigned int _nl_max_funcs;

  PetscInt _maxit; // maximum number of iterations
  PetscInt _maxf;  // maximum number of function evaluations

  // Linear solver convergence criteria
  Real _l_tol;
  Real _l_abs_tol;
  unsigned int _l_max_its;

  /// maximum number of ping-pong iterations
  unsigned int _n_nl_pingpong = 0;
  unsigned int _n_max_nl_pingpong = std::numeric_limits<unsigned int>::max();

  PetscErrorCode ierr;
  PetscInt it_petsc;
  PetscReal xnorm_petsc;
  PetscReal fnorm_petsc;
  PetscReal snorm_petsc;
};
