//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 * Function object for tests/ins/jeffery_hamel responsible for setting
 * the exact value of the velocity and pressure variables.  Inherits
 * from FunctionInterface (similarly to CompositeFunction) so that it
 * can couple to a PiecewiseLinear function which corresponds to the
 * non-dimensional semi-analytic solution on the domain [0,1].  This
 * function is responsible for scaling and mapping the semi-analytic
 * solution values to the actual velocities and pressure used in the
 * problem.
 */
class WedgeFunction : public Function, protected FunctionInterface
{
public:
  static InputParameters validParams();

  WedgeFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// The half-angle of the wedge, stored in radians.
  const Real _alpha_radians;

  /**
   * The Reynolds number, (u_max(r) * r * alpha) / nu.  Note: the
   * quantity u_max(r) * r := lambda = const.
   */
  const Real _Re;

  /// The variable (vel_x==0, vel_y==1, p==2) being computed by this instance of WedgeFunction.
  const unsigned int _var_num;

  /// The (constant) dynamic viscosity of the fluid.  Usually specified in [GlobalParams].
  const Real _mu;

  /// The (constant) density of the fluid.  Usually specified in [GlobalParams].
  const Real _rho;

  /// The kinematic viscosity = mu/rho.
  const Real _nu;

  /**
   * The constant K from the Jeffery-Hamel solution, defined by:
   * K = -f - 1/(4 * alpha^2) * (alpha * Re * f^2 + f'')
   * which is required for computing the exact pressure:
   *
   * p(r,theta) = p_star + (2 * mu * lambda) / (r^2) * (f(theta) + K)
   */
  const Real _K;

  /**
   * The quantity u_max(r) * r, which is constant for this problem,
   * and can be computed given the Reynolds number, nu, and alpha
   * according to:
   * lambda = _Re * _nu / _alpha_radians.
   */
  const Real _lambda;

  /**
   * The pressure constant, whose value is determined from the
   * pressure pin.  Here, we assume the pressure is pinned to 0 at
   * (r,theta)=(1,0).  This value would need to change if the pressure
   * was pinned to some other value.
   */
  const Real _p_star;

  /**
   * The pre-computed semi-analytic exact solution f(theta) as a
   * PiecewiseLinear function.
   */
  const Function & _f;
};
