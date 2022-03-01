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
#include "MathUtils.h"

/**
 * Newmark-Beta time integration method
 */
class NewmarkBeta : public TimeIntegrator
{
public:
  static InputParameters validParams();

  NewmarkBeta(const InputParameters & parameters);
  virtual ~NewmarkBeta();

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void computeADTimeDerivatives(DualReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        DualReal & ad_u_dotdot) const override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2, typename T3, typename T4, typename T5>
  void computeTimeDerivativeHelper(T & u_dot,
                                   const T2 & u_old,
                                   const T3 & u_dot_old,
                                   T4 & u_dotdot,
                                   const T5 & u_dotdot_old) const;

  /// Newmark time integration parameter-beta
  Real _beta;

  /// Newmark time integration parameter-gamma
  Real _gamma;

  /// Inactive time steps
  int _inactive_tsteps;

  /// solution vector for \f$ {du^dotdot}\over{du} \f$
  Real & _du_dotdot_du;
};

template <typename T, typename T2, typename T3, typename T4, typename T5>
void
NewmarkBeta::computeTimeDerivativeHelper(
    T & u_dot, const T2 & u_old, const T3 & u_dot_old, T4 & u_dotdot, const T5 & u_dotdot_old) const
{
  // compute second derivative
  // according to Newmark-Beta method
  // u_dotdot = first_term - second_term - third_term
  //       first_term = (u - u_old) / beta / dt ^ 2
  //      second_term = u_dot_old / beta / dt
  //       third_term = u_dotdot_old * (1 / 2 / beta - 1)
  u_dotdot -= u_old;
  u_dotdot *= 1.0 / _beta / _dt / _dt;
  MathUtils::addScaled(-1.0 / _beta / _dt, u_dot_old, u_dotdot);
  MathUtils::addScaled(-0.5 / _beta + 1.0, u_dotdot_old, u_dotdot);

  // compute first derivative
  // according to Newmark-Beta method
  // u_dot = first_term + second_term + third_term
  //       first_term = u_dot_old
  //      second_term = u_dotdot_old * (1 - gamma) * dt
  //       third_term = u_dotdot * gamma * dt
  u_dot = u_dot_old;
  MathUtils::addScaled((1.0 - _gamma) * _dt, u_dotdot_old, u_dot);
  MathUtils::addScaled(_gamma * _dt, u_dotdot, u_dot);
}
