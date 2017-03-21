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

#ifndef NONLINEARSYSTEM_H
#define NONLINEARSYSTEM_H

#include "SystemBase.h"
#include "NonlinearSystemBase.h"

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblemBase ;-)
 */
class NonlinearSystem : public NonlinearSystemBase
{
public:
  NonlinearSystem(FEProblemBase & problem, const std::string & name);
  virtual ~NonlinearSystem();

  virtual void solve() override;

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() override;

  /**
   * Returns the current nonlinear iteration number.  In libmesh, this is
   * updated during the nonlinear solve, so it should be up-to-date.
   */
  virtual unsigned int getCurrentNonlinearIterationNumber() override
  {
    return _transient_sys.get_current_nonlinear_iteration_number();
  }

  virtual void setupFiniteDifferencedPreconditioner() override;

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  virtual bool converged() override;

  virtual NumericVector<Number> & RHS() override { return *_transient_sys.rhs; }

  virtual NonlinearSolver<Number> * nonlinearSolver() override
  {
    return _transient_sys.nonlinear_solver.get();
  }

  virtual NumericVector<Number> & solutionOld() override
  {
    return *_transient_sys.old_local_solution;
  }

  virtual NumericVector<Number> & solutionOlder() override
  {
    return *_transient_sys.older_local_solution;
  }

  virtual TransientNonlinearImplicitSystem & sys() { return _transient_sys; }

protected:
  TransientNonlinearImplicitSystem & _transient_sys;
};

#endif /* NONLINEARSYSTEM_H */
