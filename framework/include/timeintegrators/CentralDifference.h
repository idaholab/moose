//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActuallyExplicitEuler.h"
#include "libmesh/numeric_vector.h"

/**
 * Implements a truly explicit (no nonlinear solve) Central Difference time
 * integration scheme.
 */
class CentralDifference : public ActuallyExplicitEuler
{
public:
  CentralDifference(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialSetup() override;

  virtual int order() override { return 2; }
  virtual void computeTimeDerivatives() override;
  void computeADTimeDerivatives(ADReal & ad_u_dot,
                                const dof_id_type & dof,
                                ADReal & ad_u_dotdot) const override;

protected:
  /// solution vector for \f$ {du^dotdot}\over{du} \f$
  Real & _du_dotdot_du;

  /// The older solution
  const NumericVector<Number> & _solution_older;

  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2, typename T3, typename T4>
  void
  computeTimeDerivativeHelper(T & u_dot, T2 & u_dotdot, const T3 & u_old, const T4 & u_older) const;
};