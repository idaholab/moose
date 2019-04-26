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

class ExplicitEuler;

template <>
InputParameters validParams<ExplicitEuler>();

/**
 * Explicit Euler time integrator
 */
class ExplicitEuler : public TimeIntegrator
{
public:
  ExplicitEuler(const InputParameters & parameters);

  virtual void preSolve() override;
  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof) override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2>
  void computeTimeDerivativeHelper(T & u_dot, const T2 & u_old);
};

template <typename T, typename T2>
void
ExplicitEuler::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old)
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}

