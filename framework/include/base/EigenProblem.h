/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef EIGENPROBLEM_H
#define EIGENPROBLEM_H

#include "libmesh/libmesh_config.h"


#include "FEProblemBase.h"
#include "NonlinearEigenSystem.h"

class EigenProblem;

template<>
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
#if LIBMESH_HAVE_SLEPC
  virtual bool converged() override;
#endif

  virtual unsigned int getNEigenPairsRequired() { return _n_eigen_pairs_required; }
  virtual bool isGeneralizedEigenvalueProblem() { return _generalized_eigenvalue_problem; }
  virtual void computeJacobian(const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian, Moose::KernelType kernel_type) override;
#if LIBMESH_HAVE_SLEPC
  void setEigenproblemType(Moose::EigenProblemType eigen_problem_type);
#endif
protected:
  unsigned int _n_eigen_pairs_required;
  bool _generalized_eigenvalue_problem;
  NonlinearEigenSystem * _nl_eigen;
};

#endif /* EIGENPROBLEM_H */
