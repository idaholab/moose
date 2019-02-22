//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EIGENPROBLEM_H
#define EIGENPROBLEM_H

// MOOSE Includes
#include "FEProblemBase.h"

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
  EigenProblem(const InputParameters & parameters);

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
#if LIBMESH_HAVE_SLEPC
  void setEigenproblemType(Moose::EigenProblemType eigen_problem_type);

  /**
   * Form a Jacobian matrix for all kernels and BCs with a given tag
   */
  virtual void computeJacobianTag(const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian,
                                  TagID tag) override;

  /**
   * Form two Jacobian matrices, whre each is associateed with one tag, through one
   * element-loop.
   */
  virtual void computeJacobianAB(const NumericVector<Number> & soln,
                                 SparseMatrix<Number> & jacobianA,
                                 SparseMatrix<Number> & jacobianB,
                                 TagID tagA,
                                 TagID tagB);

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
#endif
protected:
  unsigned int _n_eigen_pairs_required;
  bool _generalized_eigenvalue_problem;
  std::shared_ptr<NonlinearEigenSystem> _nl_eigen;

  /// Timers
  PerfID _compute_jacobian_tag_timer;
  PerfID _compute_jacobian_ab_timer;
  PerfID _compute_residual_tag_timer;
  PerfID _compute_residual_ab_timer;
  PerfID _solve_timer;
};

#endif /* EIGENPROBLEM_H */
