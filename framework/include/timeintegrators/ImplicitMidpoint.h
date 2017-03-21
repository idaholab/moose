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

#ifndef IMPLICITMIDPOINT_H
#define IMPLICITMIDPOINT_H

#include "TimeIntegrator.h"

class ImplicitMidpoint;

template <>
InputParameters validParams<ImplicitMidpoint>();

/**
 * Second-order Runge-Kutta (implicit midpoint) time integration.
 *
 * The Butcher tableau for this method is:
 * 1/2 | 1/2
 * ----------
 *     | 1
 *
 * The stability function for this method is:
 * R(z) = -(z + 2)/(z - 2)
 * which is the same as Crank-Nicolson.
 *
 * The method is A-stable, but not L-stable,
 * lim R(z), z->oo = -1
 *
 * Although strictly speaking this is a "one stage" method, we treat
 * the "update" step as a second stage, since in finite element
 * analysis the update step requires a mass matrix solve.  The
 * implicit midpoint method can also be written in a single stage as:
 *
 * M*y_{n+1} = M*y_n + h*f(t_n + h/2, (y_{n+1} + y_n)/2)
 *
 * However this is less convenient to implement in MOOSE, since it
 * requires evaluating the time and non-time residuals on different
 * solution vectors, and the Jacobian contributions have an extra
 * factor of 1/2.
 */
class ImplicitMidpoint : public TimeIntegrator
{
public:
  ImplicitMidpoint(const InputParameters & parameters);
  virtual ~ImplicitMidpoint();

  virtual int order() { return 2; }

  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  unsigned int _stage;

  /// Buffer to store non-time residual from the first stage.
  NumericVector<Number> & _residual_stage1;
};

#endif /* IMPLICITMIDPOINT_H */
