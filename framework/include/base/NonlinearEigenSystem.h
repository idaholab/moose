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

#ifndef NONLINEAREIGENSYSTEM_H
#define NONLINEAREIGENSYSTEM_H

#include "libmesh/libmesh_config.h"


#include "SystemBase.h"
#include "NonlinearSystemBase.h"
#include "KernelBase.h"
#include "TimeKernel.h"

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/eigen_system.h"

// forward declarations
class EigenProblem;

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblemBase ;-)
 */
class NonlinearEigenSystem
#if LIBMESH_HAVE_SLEPC
                            : public NonlinearSystemBase
#endif /* LIBMESH_HAVE_SLEPC */
{
public:
  NonlinearEigenSystem(EigenProblem & problem, const std::string & name);

#if LIBMESH_HAVE_SLEPC
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
   * @return true if converged, otherwise false
   */
  virtual bool converged() override;

  virtual NumericVector<Number> & RHS() override;

  // return the Nth converged eigenvlue <real, imag>
  virtual const std::pair<Real, Real> getNthConvergedEigenvalue(dof_id_type n);

  // return all converged eigenvalues
  virtual const std::vector<std::pair<Real, Real> > & getAllConvergedEigenvalues() { return _eigen_values; }

  virtual void addEigenKernels(MooseSharedPointer<KernelBase> kernel, THREAD_ID tid) override;

  // For eigenvalue problems (including standard and generalized), inhomogeneous (Dirichlet or Neumann)
  // boundary conditions are  not allowed.
  void checkIntegrity();

  // return the number of converged eigenvlues
  virtual const unsigned int getNumConvergedEigenvalues() { return _transient_sys.get_n_converged(); };

  virtual NonlinearSolver<Number> * nonlinearSolver() override;

  virtual NumericVector<Number> & solutionOld() override { return *_transient_sys.old_local_solution; }

  virtual NumericVector<Number> & solutionOlder() override { return *_transient_sys.older_local_solution; }

  virtual TransientEigenSystem & sys() { return _transient_sys; }
protected:
  TransientEigenSystem & _transient_sys;
  std::vector<std::pair<Real, Real> > _eigen_values;
  unsigned int _n_eigen_pairs_required;
#endif /* LIBMESH_HAVE_SLEPC */
};
#endif /* NONLINEAREIGENSYSTEM_H */
