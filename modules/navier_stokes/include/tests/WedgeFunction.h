/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef WEDGEFUNCTION_H
#define WEDGEFUNCTION_H

#include "Function.h"
#include "FunctionInterface.h"

class WedgeFunction;

template<>
InputParameters validParams<WedgeFunction>();

/**
 * Function object for tests/ins/jeffery_hamel responsible for setting
 * the inlet velocity components.  Inherits from FunctionInterface
 * (similarly to CompositeFunction) so that it can couple to a
 * PiecewiseLinear function which corresponds to the non-dimensional
 * semi-analytic solution on the domain [0,1].  This function is
 * responsible for scaling and mapping the semi-analytic solution
 * values to the actual velocities used in the problem.
 */
class WedgeFunction : public Function,
                      protected FunctionInterface
{
public:
  WedgeFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  // The half-angle of the wedge, stored in radians.
  const Real _alpha_radians;

  /// The Reynolds number, (u_max(r) * r * alpha) / nu.  Note: the
  /// quantity u_max(r) * r = const in this particular solution, so it
  /// makes sense to define a constant Re using this.
  const Real _Re;

  /// The velocity component (x==0, y==1) being computed by this function.
  const unsigned int _component;

  /// The (constant) dynamic viscosity of the fluid.  Usually specified in [GlobalParams].
  const Real _mu;

  /// The (constant) density of the fluid.  Usually specified in [GlobalParams].
  const Real _rho;

  /// The pre-computed semi-analytic exact solution f(theta) as a
  /// PiecewiseLinear function.
  Function & _f;
};

#endif
