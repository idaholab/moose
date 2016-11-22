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

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/eigen_system.h"

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
  NonlinearEigenSystem(FEProblemBase & problem, const std::string & name);
  virtual ~NonlinearEigenSystem();

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

  virtual NonlinearSolver<Number> * nonlinearSolver() override;

  virtual NumericVector<Number> & solutionOld() override { return *_transient_sys.old_local_solution; }

  virtual NumericVector<Number> & solutionOlder() override { return *_transient_sys.older_local_solution; }

  virtual TransientEigenSystem & sys() { return _transient_sys; }
protected:
  TransientEigenSystem & _transient_sys;
#endif /* LIBMESH_HAVE_SLEPC */
};
#endif /* NONLINEAREIGENSYSTEM_H */
