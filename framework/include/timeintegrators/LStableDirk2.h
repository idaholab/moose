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
 * Second order diagonally implicit Runge Kutta method (Dirk) with two stages.
 *
 * The Butcher tableau for this method is:
 * alpha | alpha
 * 1     | 1-alpha alpha
 * ---------------------
 *       | 1-alpha alpha
 *
 * where alpha = 1 - sqrt(2)/2 ~ .29289
 *
 * The stability function for this method is:
 * R(z) = 4*(-z*(-sqrt(2) + 2) + z + 1) / (z**2*(-sqrt(2) + 2)**2 - 4*z*(-sqrt(2) + 2) + 4)
 *
 * The method is L-stable:
 * lim R(z), z->oo = 0
 *
 * Notes: This method is derived in detail in: R. Alexander,
 * "Diagonally implicit Runge-Kutta Methods for Stiff ODEs", SIAM
 * J. Numer. Anal., 14(6), Dec. 1977, pg. 1006-1021.  This method is
 * more expensive than Crank-Nicolson, but has the advantage of being
 * L-stable (the same type of stability as the implicit Euler method)
 * so may be more suitable for "stiff" problems.
 */
class LStableDirk2 : public TimeIntegrator
{
public:
  static InputParameters validParams();

  LStableDirk2(const InputParameters & parameters);

  virtual int order() override { return 2; }
  virtual void computeTimeDerivatives() override;
  void computeADTimeDerivatives(ADReal & ad_u_dot,
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

  //! Indicates the current stage (1 or 2).
  unsigned int _stage;

  //! Buffer to store non-time residual from first stage solve.
  NumericVector<Number> * _residual_stage1;

  //! Buffer to store non-time residual from second stage solve
  NumericVector<Number> * _residual_stage2;

  // The parameter of the method, set at construction time and cannot be changed.
  const Real _alpha;
};

template <typename T, typename T2>
void
LStableDirk2::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}
