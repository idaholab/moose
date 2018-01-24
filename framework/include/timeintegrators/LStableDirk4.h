/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef LSTABLEDIRK4_H
#define LSTABLEDIRK4_H

#include "TimeIntegrator.h"

class LStableDirk4;

template <>
InputParameters validParams<LStableDirk4>();

/**
 * Fourth-order diagonally implicit Runge Kutta method (Dirk) with five stages.
 *
 * The Butcher tableau for this method is:
 * 1/4 |  1/4
 * 0   | -1/4 1/4
 * 1/2 |  1/8 1/8 1/4
 * 1   | -3/2 3/4 3/2   1/4
 * 1   |    0 1/6 2/3 -1/12 1/4
 * ----------------------------
 * 1   |    0 1/6 2/3 -1/12 1/4
 *
 *
 * The stability function for this method is:
 * R(z) = -(28*z**4 + 32*z**3 - 384*z**2 - 768*z + 3072)/
 *         (3*z**5 - 60*z**4 + 480*z**3 - 1920*z**2 + 3840*z - 3072)
 *
 * The method is L-stable:
 * lim R(z), z->oo = 0
 *
 * Notes:
 * I found the method here:
 *
 * L. M. Skvortsov, "Diagonally implicit Runge-Kutta Methods
 * for Stiff Problems", Computational Mathematics and Mathematical
 * Physics vol 46, no 12, pp. 2110-2123, 2006.
 *
 * but it may not be the original source.  There is also a 4th-order
 * rule with 5 stages on page 107 of:
 *
 * E. Hairer and G. Wanner, Solving Ordinary Differential Equations,
 * Vol. 2: Stiff and Differential-Algebraic Problems (Springer, Berlin,
 * 1987-1991; Mir, Moscow, 1999).
 *
 * but its coefficients have less favorable "amplification factors"
 * than the present rule.
 */
class LStableDirk4 : public TimeIntegrator
{
public:
  LStableDirk4(const InputParameters & parameters);
  virtual ~LStableDirk4();

  virtual int order() { return 4; }
  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  // Indicates the current stage.
  unsigned int _stage;

  // The number of stages in the method.  According to S9.4.2/4 of the
  // standard, we can specify a constant initializer like this for
  // integral types, it does not have to appear outside the class
  // definition.
  static const unsigned int _n_stages = 5;

  // Store pointers to the various stage residuals
  NumericVector<Number> * _stage_residuals[_n_stages];

  // Butcher tableau "C" parameters derived from _gamma
  static const Real _c[_n_stages];

  // Butcher tableau "A" values derived from _gamma.  We only use the
  // lower triangle of this.
  static const Real _a[_n_stages][_n_stages];
};

#endif // LSTABLEDIRK4_H
