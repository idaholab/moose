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

class LStableDirk4;

/**
 * Fourth-order diagonally implicit Runge Kutta method (Dirk) with
 * three stages plus an update.
 *
 * The Butcher tableau for this method is:
 * gamma   | gamma                   0                          0
 * 1/2     | 1/2-gamma               gamma                      0
 * 1-gamma | 2*gamma                 1-4*gamma                  gamma
 * --------|-------------------------------------------------------------------------
 *         | 1/(24*(1/2-gamma)**2)   1 - 1/(12*(1/2-gamma)**2)  1/(24*(1/2-gamma)**2)
 *
 * where gamma = 1/2 + sqrt(3)/3 * cos(pi/18) ~ 1.06857902130162881
 *
 * The stability function for this method is:
 * R(z) = (-0.76921266689461*z**3 - 0.719846311954034*z**2 + 2.20573706179581*z - 1.0)/
 *        (1.22016884572716*z**3 - 3.42558337748497*z**2 + 3.20573706551682*z - 1.0)
 *
 * The method is *not* L-stable, it is only A-stable:
 # lim R(z), z->oo = -0.630414937726258
 *
 * Notes:
 * 1.) Method is originally due to:
 *     M. Crouzeix, "Sur l'approximation des equations differentielles
 *     operationelles lineaires par des methodes de Runge Kutta",
 *     Ph.D. thesis, Universite Paris VI, Paris, 1975.
 * 2.) Since gamma is slightly larger than 1, the first stage involves
 *     evaluation of the non-time residuals for t > t_n+dt, while the
 *     third stage involves evaluation of the non-time residual for t
 *     < t_n, which may present an issue for the first timestep (if
 *     e.g. material properties or forcing functions are not defined
 *     for t<0. We could handle this by using an alternate (more
 *     expensive) method in the first timestep, or by using a
 *     lower-order method for the first timestep and then switching to
 *     this method for subsequent timesteps.
 */
class AStableDirk4 : public TimeIntegrator
{
public:
  static InputParameters validParams();

  AStableDirk4(const InputParameters & parameters);

  virtual int order() override { return 4; }
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

  // Indicates the current stage.
  unsigned int _stage;

  // Store pointers to the various stage residuals
  NumericVector<Number> * _stage_residuals[3];

  // The parameter of the method, set at construction time and cannot be changed.
  const Real _gamma; // 1.06857902130162881

  // Butcher tableau "C" parameters derived from _gamma
  // 1.06857902130162881, 0.5, -.06857902130162881
  Real _c[3];

  // Butcher tableau "A" values derived from _gamma.  We only use the
  // lower triangle of this.
  // 1.06857902130162881
  // -.56857902130162881, 1.06857902130162881
  // 2.13715804260325762, -3.27431608520651524, 1.06857902130162881
  Real _a[3][3];

  // The Butcher tableau "b" parameters derived from _gamma;
  // 1.2888640051572051e-01, 7.4222719896855893e-01, 1.2888640051572051e-01
  Real _b[3];

  // If true, we use a more expensive method (LStableDirk4) to
  // "bootstrap" the first timestep of this method and avoid
  // evaluating residuals before the initial time.
  bool _safe_start;

  // A pointer to the "bootstrapping" method to use if _safe_start==true.
  std::shared_ptr<LStableDirk4> _bootstrap_method;
};

template <typename T, typename T2>
void
AStableDirk4::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}
