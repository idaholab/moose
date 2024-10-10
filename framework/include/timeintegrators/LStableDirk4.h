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
 * Fourth-order diagonally implicit Runge Kutta method (Dirk) with five stages.
 *
 * The Butcher tableau for this method is:
 * 1/4 |  1/4
 * 0   | -1/4 1/4
 * 1/2 |  1/8 1/8 1/4
 * 1   | -3/2 3/4 3/2   1/4
 * 1   |    0 1/6 2/3 -1/12 1/4
 * ----------------------------
 * 1   |    0 1/6 2/3 -1/12 1/4
 *
 *
 * The stability function for this method is:
 * R(z) = -(28*z**4 + 32*z**3 - 384*z**2 - 768*z + 3072)/
 *         (3*z**5 - 60*z**4 + 480*z**3 - 1920*z**2 + 3840*z - 3072)
 *
 * The method is L-stable:
 * lim R(z), z->oo = 0
 *
 * Notes:
 * I found the method here:
 *
 * L. M. Skvortsov, "Diagonally implicit Runge-Kutta Methods
 * for Stiff Problems", Computational Mathematics and Mathematical
 * Physics vol 46, no 12, pp. 2110-2123, 2006.
 *
 * but it may not be the original source.  There is also a 4th-order
 * rule with 5 stages on page 107 of:
 *
 * E. Hairer and G. Wanner, Solving Ordinary Differential Equations,
 * Vol. 2: Stiff and Differential-Algebraic Problems (Springer, Berlin,
 * 1987-1991; Mir, Moscow, 1999).
 *
 * but its coefficients have less favorable "amplification factors"
 * than the present rule.
 */
class LStableDirk4 : public TimeIntegrator
{
public:
  static InputParameters validParams();

  LStableDirk4(const InputParameters & parameters);

  virtual int order() override { return 4; }
  virtual void computeTimeDerivatives() override;
  virtual void computeADTimeDerivatives(ADReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        ADReal & ad_u_dotdot) const override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual bool overridesSolve() const override { return true; }

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2>
  void computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const;

  // Indicates the current stage.
  unsigned int _stage;

  // The number of stages in the method.  According to S9.4.2/4 of the
  // standard, we can specify a constant initializer like this for
  // integral types, it does not have to appear outside the class
  // definition.
  static const unsigned int _n_stages = 5;

  // Store pointers to the various stage residuals
  NumericVector<Number> * _stage_residuals[_n_stages];

  // Butcher tableau "C" parameters derived from _gamma
  static const Real _c[_n_stages];

  // Butcher tableau "A" values derived from _gamma.  We only use the
  // lower triangle of this.
  static const Real _a[_n_stages][_n_stages];
};

template <typename T, typename T2>
void
LStableDirk4::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}
