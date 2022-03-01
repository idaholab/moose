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
 * Base class for three different explicit second-order Runge-Kutta
 * time integration methods:
 * - Explicit midpoint method (ExplicitMidpoint.C)
 * - Heun's method (Heun.C)
 * - Ralston's method (Ralston.C)
 *
 * Each of these methods is characterized by the following generic
 * Butcher tableau:
 * 0   | 0
 * a   | a   0
 * ---------------------
 *     | b1  b2
 *
 * where a, b1, and b2 are constants that define the different
 * methods.
 *
 * The stability function for each of these methods is:
 * R(z) = z*(z + 2)/2 + 1
 *
 * The methods are all explicit, so they are neither A-stable nor L-stable,
 * lim R(z), z->oo = oo
 *
 * See also:
 *   https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
 *   https://en.wikipedia.org/wiki/Midpoint_method
 *   https://en.wikipedia.org/wiki/Heun%27s_method
 *
 * All three methods require two mass matrix (linear) system solves
 * per timestep. Although strictly speaking these are "two stage"
 * methods, we treat the "update" step as a third stage, since in
 * finite element analysis the update step requires a mass matrix
 * solve.
 *
 * IMPORTANT: To use the explicit TimeIntegrators derived from this
 * method, you must generally add "implicit=false" to the Kernels,
 * Materials, etc. used in your simulation, so that MOOSE evaluates
 * them correctly!  An important exception are TimeDerivative kernels,
 * which should never be marked "implicit=false".
 */
class ExplicitRK2 : public TimeIntegrator
{
public:
  static InputParameters validParams();

  ExplicitRK2(const InputParameters & parameters);

  virtual void preSolve() override;
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
  template <typename T, typename T2, typename T3>
  void computeTimeDerivativeHelper(T & u_dot, const T2 & u_old, const T3 & u_older) const;

  unsigned int _stage;

  /// Buffer to store non-time residual from the first stage.
  NumericVector<Number> & _residual_old;

  /// The older solution
  const NumericVector<Number> & _solution_older;

  /// The method coefficients.  See the table above for description.
  /// These are pure virtual in the base class, and must be overridden
  /// in subclasses to implement different schemes.
  virtual Real a() const = 0;
  virtual Real b1() const = 0;
  virtual Real b2() const = 0;
};

template <typename T, typename T2, typename T3>
void
ExplicitRK2::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old, const T3 & u_older) const
{
  if (_stage < 3)
    u_dot -= u_old;
  else
    u_dot -= u_older;

  u_dot *= 1. / _dt;
}
