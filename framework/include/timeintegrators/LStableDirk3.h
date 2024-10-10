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
 * Third order diagonally implicit Runge Kutta method (Dirk) with three stages.
 *
 * The Butcher tableau for this method is:
 * gamma       | gamma
 * (1+gamma)/2 | (1-gamma)/2                         gamma
 * 1           | (1/4)*(-6*gamma**2 + 16*gamma - 1)  (1/4)*(6*gamma**2 - 20*gamma + 5)  gamma
 * ------------------------------------------------------------------------------------------
 *             | (1/4)*(-6*gamma**2 + 16*gamma - 1)  (1/4)*(6*gamma**2 - 20*gamma + 5)  gamma
 *
 * where gamma = -sqrt(2)*cos(atan(sqrt(2)/4)/3)/2 + sqrt(6)*sin(atan(sqrt(2)/4)/3)/2 + 1  ~
 * .435866521508459
 *
 * The stability function for this method is:
 * R(z) = (1.90128552647780115*z**2 + 2.46079651620301599*z - 8) /
 *        (0.662446064957040178*z**3 - 4.55951098972521484*z**2 + 10.460796516203016*z - 8)
 *
 * The method is L-stable:
 * lim R(z), z->oo = 0
 *
 * Notes: This method is derived in detail in: R. Alexander,
 * "Diagonally implicit Runge-Kutta Methods for Stiff ODEs", SIAM
 * J. Numer. Anal., 14(6), Dec. 1977, pg. 1006-1021.  Unlike BDF3,
 * this method is L-stable and so may be more suitable for "stiff"
 * problems.
 */
class LStableDirk3 : public TimeIntegrator
{
public:
  static InputParameters validParams();

  LStableDirk3(const InputParameters & parameters);

  virtual int order() override { return 3; }
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

  // Store pointers to the various stage residuals
  NumericVector<Number> * _stage_residuals[3];

  // The parameter of the method, set at construction time and cannot be changed.
  const Real _gamma; // 0.4358665215084589

  // Butcher tableau "C" parameters derived from _gamma
  // 0.4358665215084589, 0.7179332607542295, 1.0000000000000000
  Real _c[3];

  // Butcher tableau "A" values derived from _gamma.  We only use the
  // lower triangle of this.
  // 0.4358665215084589
  // 0.2820667392457705,  0.4358665215084589
  // 1.2084966491760099, -0.6443631706844688, 0.4358665215084589
  Real _a[3][3];
};

template <typename T, typename T2>
void
LStableDirk3::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}
