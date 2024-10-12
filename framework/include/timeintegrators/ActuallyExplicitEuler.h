//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitTimeIntegrator.h"

/**
 * Implements a truly explicit (no nonlinear solve) first-order, forward Euler
 * time integration scheme.
 */
class ActuallyExplicitEuler : public ExplicitTimeIntegrator
{
public:
  static InputParameters validParams();

  ActuallyExplicitEuler(const InputParameters & parameters);

  virtual int order() override { return 1; }
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

  const bool & _constant_mass;
};

template <typename T, typename T2>
void
ActuallyExplicitEuler::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}
