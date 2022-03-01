//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeIntegrator.h"

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
  static InputParameters validParams();

  ImplicitMidpoint(const InputParameters & parameters);

  virtual int order() override { return 2; }

  virtual void computeTimeDerivatives() override;
  void computeADTimeDerivatives(DualReal & ad_u_dot,
                                const dof_id_type & dof,
                                DualReal & ad_u_dotdot) const override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2>
  void computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const;

  unsigned int _stage;

  /// Buffer to store non-time residual from the first stage.
  NumericVector<Number> & _residual_stage1;
};

template <typename T, typename T2>
void
ImplicitMidpoint::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}
