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

#ifndef TIMEINTEGRATOR_H
#define TIMEINTEGRATOR_H

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"

// Forward declarations
class TimeIntegrator;
class FEProblemBase;
class SystemBase;
class NonlinearSystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<TimeIntegrator>();

/**
 * Base class for time integrators
 *
 * Time integrators fulfill two functions:
 * 1) computing u_dot vector (used for computing time derivatives in kernels) and its derivative
 * 2) combining the residual vectors into the final one
 *
 * Capability (1) is used by both NonlinearSystem and AuxiliarySystem, while (2) can be obviously
 * used
 * only by NonlinearSystem (AuxiliarySystem does not produce residual).
 */
class TimeIntegrator : public MooseObject, public Restartable
{
public:
  TimeIntegrator(const InputParameters & parameters);
  virtual ~TimeIntegrator();

  virtual void preSolve() {}
  virtual void preStep() {}
  virtual void solve();

  /**
   * Callback to the TimeIntegrator called immediately after the
   * residuals are computed in NonlinearSystem::computeResidual() (it
   * is not really named well...).  The residual vector which is
   * passed in to this function should be filled in by the user with
   * the _Re_time and _Re_non_time vectors in a way that makes sense
   * for the particular TimeIntegration method.
   */
  virtual void postStep(NumericVector<Number> & /*residual*/) {}

  /**
   * Callback to the TimeIntegrator called immediately after
   * TimeIntegrator::solve() (so the name does make sense!).  See
   * e.g. CrankNicolson for an example of what can be done in the
   * postSolve() callback -- there it is used to move the residual
   * from the "old" timestep forward in time to avoid recomputing it.
   */
  virtual void postSolve() {}

  virtual int order() = 0;
  virtual void computeTimeDerivatives() = 0;

protected:
  FEProblemBase & _fe_problem;
  SystemBase & _sys;
  NonlinearSystemBase & _nl;

  /// solution vector for u^dot
  NumericVector<Number> & _u_dot;
  /// solution vector for \f$ {du^dot}\over{du} \f$
  Real & _du_dot_du;
  /// solution vectors
  const NumericVector<Number> *& _solution;
  const NumericVector<Number> & _solution_old;
  const NumericVector<Number> & _solution_older;
  //
  int & _t_step;
  //
  Real & _dt;
  Real & _dt_old;

  /// residual vector for time contributions
  NumericVector<Number> & _Re_time;
  /// residual vector for non-time contributions
  NumericVector<Number> & _Re_non_time;
};

#endif /* TIMEINTEGRATOR_H */
