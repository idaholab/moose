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

class MooseEigenSystem;
class FEProblemBase;

/**
 * This class provides reusable routines for eigenvalue executioners.
 */
class EigenExecutionerBase : public Executioner
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  static InputParameters validParams();

  EigenExecutionerBase(const InputParameters & parameters);

  virtual void init() override;

  /**
   * The old eigenvalue used by inverse power iterations
   */
  const Real & eigenvalueOld();

  /**
   * Normalize solution so that |Bx| = k
   */
  virtual void makeBXConsistent(Real k);

  /**
   * Make sure time kernel is not presented
   */
  virtual void checkIntegrity();

  /**
   * Perform inverse power iterations with the initial guess of the solution
   *
   * @param min_iter The minimum number of power iterations.
   * @param max_iter The maximum number of power iterations.
   * @param pfactor The factor on reducing the residual norm of each power iteration.
   * @param cheb_on To turn the Chebyshev acceleration on.
   * @param tol_eig Tolerance on the difference of the eigenvalue of two successive iterations.
   * @param echo True to make screen printouts.
   * @param xdiff Name of the postprocessor evaluating the difference of the solution norm of two
   * successive iterations.
   * @param tol_x Tolerance on the difference of the solution norm of two successive iterations.
   * @param k Eigenvalue, input as the initial guess.
   * @param initial_res The initial residual.
   * @return true solve converges, otherwise false.
   */
  virtual bool inversePowerIteration(unsigned int min_iter,
                                     unsigned int max_iter,
                                     Real pfactor,
                                     bool cheb_on,
                                     Real tol_eig,
                                     bool echo,
                                     PostprocessorName xdiff,
                                     Real tol_x,
                                     Real & k,
                                     Real & initial_res);

  /**
   * Override this for actions that should take place before linear solve of each inverse power
   * iteration
   */
  virtual void preIteration();

  /**
   * Override this for actions that should take place after linear solve of each inverse power
   * iteration
   */
  virtual void postIteration();

  /**
   * Override this for actions that should take place after the main solve
   */
  virtual void postExecute() override;

  /**
   * Normalize the solution vector based on the postprocessor value for normalization
   *
   * @param force Force the re-evaluation of the postprocessor for normalization.
   * Returns the scaling factor just applied.
   */
  virtual Real normalizeSolution(bool force = true);

  /**
   * Perform nonlinear solve with the initial guess of the solution
   *
   * @param rel_tol Relative tolerance on system residual.
   * @param abs_tol Absolute tolerance on system residual.
   * @param pfactor The factor on reducing the residual norm of each linear iteration.
   * @param k Eigenvalue, input as the initial guess.
   * @return true solve converges, otherwise false.
   */
  virtual bool nonlinearSolve(Real rel_tol, Real abs_tol, Real pfactor, Real & k);

  /**
   * A method for returning the eigenvalue computed by the executioner
   * @return A reference to the eigenvalue stored withing the executioner
   */
  Real & eigenValue() { return _eigenvalue; }

protected:
  /**
   * Print eigenvalue
   */
  virtual void printEigenvalue();

  // the fe problem
  FEProblemBase & _problem;
  MooseEigenSystem & _eigen_sys;

  /// dummy solve object for properly setting PETSc options
  FEProblemSolve _feproblem_solve;

  /// Storage for the eigenvalue computed by the executioner
  PostprocessorValue & _eigenvalue;

  // postprocessor for eigenvalue
  const Real & _source_integral;
  Real _source_integral_old;

  /// Postprocessor for normalization
  const Real & _normalization;
  ExecFlagEnum _norm_exec;

  // Chebyshev acceleration
  class Chebyshev_Parameters
  {
  public:
    Chebyshev_Parameters();
    void reinit();

    const unsigned int n_iter;  // minimum number of accelerated iteration each cycle
    const unsigned int fsmooth; // number of unaccelerated iteration each cycle
    unsigned int finit;         // number of unaccelerated iteration before Chebyshev

    unsigned int lgac;          // doing acceleration or not
    unsigned int icheb;         // number of acceleration in current cycle
    unsigned int iter_begin;    // starting number of current acceleration cycle
    double error_begin;         // starting flux error of current acceleration cycle
    double flux_error_norm_old; // flux error of previous power iteration
    double ratio;               // estimation of dominant ratio
    double ratio_new;           // new estimated dominant ratio
    unsigned int icho;          // improved ratio estimation
  };
  void chebyshev(Chebyshev_Parameters & params,
                 unsigned int iter,
                 const PostprocessorValue * solution_diff);
};
