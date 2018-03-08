//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NONLINEAREIGENSYSTEM_H
#define NONLINEAREIGENSYSTEM_H

#include "libmesh/libmesh_config.h"

#include "NonlinearSystemBase.h"

#include "libmesh/transient_system.h"
#include "libmesh/eigen_system.h"

// forward declarations
class EigenProblem;
class KernelBase;

#if LIBMESH_HAVE_SLEPC

/**
 * Nonlinear eigenvalue system to be solved
 */
class NonlinearEigenSystem : public NonlinearSystemBase
{
public:
  NonlinearEigenSystem(EigenProblem & problem, const std::string & name);

  virtual void solve() override;

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() override;

  /**
   * Returns the current nonlinear iteration number.  In libmesh, this is
   * updated during the nonlinear solve, so it should be up-to-date.
   */
  virtual unsigned int getCurrentNonlinearIterationNumber() override;

  virtual void setupFiniteDifferencedPreconditioner() override;

  /**
   * Returns the convergence state
   *
   * @return true if converged, otherwise false
   */
  virtual bool converged() override;

  virtual NumericVector<Number> & RHS() override;

  virtual void addEigenKernels(std::shared_ptr<KernelBase> kernel, THREAD_ID tid) override;

  /**
   * Get the number of converged eigenvalues
   *
   * @return The number of converged eigenvalues
   */
  virtual unsigned int getNumConvergedEigenvalues() const
  {
    return _transient_sys.get_n_converged();
  };

  virtual NonlinearSolver<Number> * nonlinearSolver() override;

  virtual NumericVector<Number> & solutionOld() override
  {
    return *_transient_sys.old_local_solution;
  }

  virtual NumericVector<Number> & solutionOlder() override
  {
    return *_transient_sys.older_local_solution;
  }

  virtual TransientEigenSystem & sys() { return _transient_sys; }

  /**
   * For eigenvalue problems (including standard and generalized), inhomogeneous (Dirichlet or
   * Neumann)
   * boundary conditions are not allowed.
   */
  void checkIntegrity();

  /**
   * Return the Nth converged eigenvalue.
   *
   * @return The Nth converged eigenvalue as a complex number, i.e. the first and the second number
   * is the real and the imaginary part of
   * the eigenvalue, respectively.
   */
  virtual const std::pair<Real, Real> getNthConvergedEigenvalue(dof_id_type n);

  /**
   * Get the number of converged eigenvalues
   *
   * @return all converged eigenvalues as complex numbers
   */
  virtual const std::vector<std::pair<Real, Real>> & getAllConvergedEigenvalues()
  {
    return _eigen_values;
  }

protected:
  TransientEigenSystem & _transient_sys;
  EigenProblem & _eigen_problem;
  std::vector<std::pair<Real, Real>> _eigen_values;
  unsigned int _n_eigen_pairs_required;
};

#else

class NonlinearEigenSystem : public libMesh::ParallelObject
{
public:
  NonlinearEigenSystem(EigenProblem & problem, const std::string & name);

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  bool converged() { return false; }

  void checkIntegrity() {}
};

#endif

#endif /* NONLINEAREIGENSYSTEM_H */
