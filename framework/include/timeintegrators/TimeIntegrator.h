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

#include "MooseObject.h"
#include "libmesh/numeric_vector.h"
#include "Restartable.h"

class TimeIntegrator;
class FEProblem;
class SystemBase;
class NonlinearSystem;

template<>
InputParameters validParams<TimeIntegrator>();

/**
 * Base class for time integrators
 *
 * Time integrators fulfill two functions:
 * 1) computing u_dot vector (used for computing time derivatives in kernels) and its derivative
 * 2) combining the residual vectors into the final one
 *
 * Capability (1) is used by both NonlinearSystem and AuxiliarySystem, while (2) can be obviously used
 * only by NonlinearSystem (AuxiliarySystem does not produce residual).
 */
class TimeIntegrator :
  public MooseObject,
  public Restartable
{
public:
  TimeIntegrator(const std::string & name, InputParameters parameters);
  virtual ~TimeIntegrator();

  virtual void preSolve() { }
  virtual void preStep() { }
  virtual void solve();
  virtual void postStep(NumericVector<Number> & /*residual*/) { }
  virtual void postSolve() { }

  virtual int order() = 0;
  virtual void computeTimeDerivatives() = 0;

protected:

  FEProblem & _fe_problem;
  SystemBase & _sys;
  NonlinearSystem & _nl;

  /// solution vector for u^dot
  NumericVector<Number> & _u_dot;
  /// solution vector for \f$ {du^dot}\over{du} \f$
  Real & _du_dot_du;
  /// solution vectors
  const NumericVector<Number> * & _solution;
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
