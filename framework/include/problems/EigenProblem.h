//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE Includes
#include "FEProblemBase.h"
#include "Eigenvalue.h"

// Forward declarations
class EigenProblem;
class NonlinearEigenSystem;

template <>
InputParameters validParams<EigenProblem>();

/**
 * Problem for solving eigenvalue problems
 */
class EigenProblem : public FEProblemBase
{
public:
  static InputParameters validParams();

  EigenProblem(const InputParameters & parameters);

  virtual void solve() override;

  virtual void init() override;

  virtual bool converged() override;

  virtual unsigned int getNEigenPairsRequired() { return _n_eigen_pairs_required; }
  virtual void setNEigenPairsRequired(unsigned int n_eigen_pairs)
  {
    _n_eigen_pairs_required = n_eigen_pairs;
  }
  virtual bool isGeneralizedEigenvalueProblem() { return _generalized_eigenvalue_problem; }
  virtual bool isNonlinearEigenvalueSolver();
  // silences warning in debug mode about the other computeJacobian signature being hidden
  using FEProblemBase::computeJacobian;

  NonlinearEigenSystem & getNonlinearEigenSystem() { return *_nl_eigen; }

  virtual void checkProblemIntegrity() override;

  /**
   * A flag indicates if a negative sign is used in eigen kernels.
   * If the negative sign is used, eigen kernels are consistent in nonlinear solver.
   * In nonlinear solver, RHS kernels always have a negative sign.
   */
  bool negativeSignEigenKernel() { return _negative_sign_eigen_kernel; }

  /**
   * If we need to initialize eigen vector. We initialize the eigen vector
   * only when "auto_initialization" is on and nonlinear eigen solver is selected.
   */
  bool needInitializeEigenVector();

  /*
   * Specify whether or not to initialize eigenvector automatically
   */
  void needInitializeEigenVector(bool need) { _auto_initilize_eigen_vector = need; }

#if LIBMESH_HAVE_SLEPC
  void setEigenproblemType(Moose::EigenProblemType eigen_problem_type);

  virtual Real computeResidualL2Norm() override;

  /**
   * Form a Jacobian matrix for all kernels and BCs with a given tag
   */
  virtual void computeJacobianTag(const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian,
                                  TagID tag) override;

  /**
   * Form several matrices simultaneously
   */
  void computeMatricesTags(const NumericVector<Number> & soln,
                           const std::vector<std::unique_ptr<SparseMatrix<Number>>> & jacobians,
                           const std::set<TagID> & tags);

  /**
   * Form two Jacobian matrices, whre each is associateed with one tag, through one
   * element-loop.
   */
  virtual void computeJacobianAB(const NumericVector<Number> & soln,
                                 SparseMatrix<Number> & jacobianA,
                                 SparseMatrix<Number> & jacobianB,
                                 TagID tagA,
                                 TagID tagB);

  virtual void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks) override;

  /**
   * Form a vector for all kernels and BCs with a given tag
   */
  virtual void computeResidualTag(const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual,
                                  TagID tag) override;

  /**
   * Form two vetors, whre each is associateed with one tag, through one
   * element-loop.
   */
  virtual void computeResidualAB(const NumericVector<Number> & soln,
                                 NumericVector<Number> & residualA,
                                 NumericVector<Number> & residualB,
                                 TagID tagA,
                                 TagID tagB);

  /**
   * Scale eigenvector. Scaling_factor is often computed based on physics.
   */
  void scaleEigenvector(const Real scaling_factor);

  /**
   * For nonlinear eigen solver, a good initial value can help convergence.
   * Should set initial values for only eigen variables.
   */
  void initEigenvector(const Real initial_value);

  /**
   * Whether or not we do free power iteration. It is used in convergence check.
   * We need to mark the solver as "converged" when doing free power to retrieve
   * the final solution from SLPEc
   */
  bool doFreePowerIteration() { return _do_free_power_iteration; }

  /**
   * Set a flag to indicate whether or not we do free power iteration.
   */
  void doFreePowerIteration(bool do_power) { _do_free_power_iteration = do_power; }

  /**
   * Which eigenvalue is active
   */
  unsigned int activeEigenvalueIndex() { return _active_eigen_index; }

  /**
   * Return console handle, and we use that in EPSMonitor to print out eigenvalue
   */
  const ConsoleStream & console() { return _console; }

  /**
   * Hook up monitors for SNES and KSP
   */
  virtual void initPetscOutput() override;

  /**
   * Whether or not to output eigenvalue inverse. The inverse is useful for
   * neutronics community
   */
  bool outputInverseEigenvalue() { return _output_inverse_eigenvalue; }

  /**
   * Set a flag to indicate whether or not to output eigenvalue inverse.
   */
  void outputInverseEigenvalue(bool inverse) { _output_inverse_eigenvalue = inverse; }

#endif

protected:
  unsigned int _n_eigen_pairs_required;
  bool _generalized_eigenvalue_problem;
  std::shared_ptr<NonlinearEigenSystem> _nl_eigen;

  bool _negative_sign_eigen_kernel;

  unsigned int _active_eigen_index;

  bool _auto_initilize_eigen_vector;
  bool _do_free_power_iteration;
  bool _output_inverse_eigenvalue;

  /// Timers
  PerfID _compute_jacobian_tag_timer;
  PerfID _compute_jacobian_ab_timer;
  PerfID _compute_residual_tag_timer;
  PerfID _compute_residual_ab_timer;
  PerfID _solve_timer;
  PerfID _compute_jacobian_blocks_timer;
};
