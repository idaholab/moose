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
 * Crank-Nicolson time integrator.
 *
 * The scheme is defined as:
 *   \f$ \frac{du}{dt} = 1/2 * (F(U^{n+1}) + F(U^{n})) \f$,
 * but the form we are using it in is:
 *   \f$ 2 * \frac{du}{dt} = (F(U^{n+1}) + F(U^{n})) \f$.
 */
class CrankNicolson : public TimeIntegrator
{
public:
  static InputParameters validParams();

  CrankNicolson(const InputParameters & parameters);

  virtual void init() override;
  virtual int order() override { return 2; }
  virtual void computeTimeDerivatives() override;
  void computeADTimeDerivatives(DualReal & ad_u_dot,
                                const dof_id_type & dof,
                                DualReal & ad_u_dotdot) const override;
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual void postStep() override;

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2>
  void computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const;

  NumericVector<Number> & _residual_old;
};

template <typename T, typename T2>
void
CrankNicolson::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old) const
{
  u_dot -= u_old;
  u_dot *= 2. / _dt;
}
