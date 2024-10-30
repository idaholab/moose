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
 * BDF2 time integrator
 */
class BDF2 : public TimeIntegrator
{
public:
  static InputParameters validParams();

  BDF2(const InputParameters & parameters);

  virtual int order() override { return 2; }
  virtual void preStep() override;
  virtual void computeTimeDerivatives() override;
  void computeADTimeDerivatives(ADReal & ad_u_dot,
                                const dof_id_type & dof,
                                ADReal & ad_u_dotdot) const override;
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual bool overridesSolve() const override { return false; }

protected:
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2, typename T3, typename T4>
  void
  computeTimeDerivativeHelper(T & u_dot, const T2 & u, const T3 & u_old, const T4 & u_older) const;

  virtual Real duDotDuCoeff() const override;

  std::vector<Real> & _weight;

  /// The older solution
  const NumericVector<Number> & _solution_older;
};

namespace BDF2Helper
{
}

template <typename T, typename T2, typename T3, typename T4>
void
BDF2::computeTimeDerivativeHelper(T & u_dot,
                                  const T2 & u,
                                  const T3 & u_old,
                                  const T4 & u_older) const
{
  if (_t_step == 1)
  {
    u_dot -= u_old;
    u_dot *= 1 / _dt;
  }
  else
  {
    MathUtils::addScaled(_weight[0], u, u_dot);
    MathUtils::addScaled(_weight[1], u_old, u_dot);
    MathUtils::addScaled(_weight[2], u_older, u_dot);
    u_dot *= 1. / _dt;
  }
}
