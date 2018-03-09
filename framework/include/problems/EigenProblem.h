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
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class EigenProblem : public FEProblemBase
{
public:
  EigenProblem(const InputParameters & parameters);

  virtual ~EigenProblem();

  virtual void solve() override;

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
#if LIBMESH_HAVE_SLEPC
  void setEigenproblemType(Moose::EigenProblemType eigen_problem_type);

  virtual void computeJacobian(const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian,
                               TagID tag) override;

  void computeJacobianAB(const NumericVector<Number> & soln,
                         SparseMatrix<Number> & jacobianA,
                         SparseMatrix<Number> & jacobianB,
                         TagID tagA,
                         TagID tagB);

  virtual void computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual,
                               TagID tag) override;

  void computeResidualAB(const NumericVector<Number> & soln,
                         NumericVector<Number> & residualA,
                         NumericVector<Number> & residualB,
                         TagID tagA,
                         TagID tagB);
#endif
protected:
  unsigned int _n_eigen_pairs_required;
  bool _generalized_eigenvalue_problem;
  std::shared_ptr<NonlinearEigenSystem> _nl_eigen;
};

#endif /* EIGENPROBLEM_H */
