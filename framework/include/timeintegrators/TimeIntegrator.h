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
class NonlinearSystem;

template<>
InputParameters validParams<TimeIntegrator>();

/**
 * Base class for time integrators
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
  NonlinearSystem & _nl;

  /// solution vector for u^dot
  NumericVector<Number> & _u_dot;
  /// solution vector for {du^dot}\over{du}
  NumericVector<Number> & _du_dot_du;
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
