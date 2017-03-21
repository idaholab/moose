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

#ifndef LSTABLEDIRK3_H
#define LSTABLEDIRK3_H

#include "TimeIntegrator.h"

class LStableDirk3;

template <>
InputParameters validParams<LStableDirk3>();

/**
 * Third order diagonally implicit Runge Kutta method (Dirk) with three stages.
 *
 * The Butcher tableau for this method is:
 * gamma       | gamma
 * (1+gamma)/2 | (1-gamma)/2                         gamma
 * 1           | (1/4)*(-6*gamma**2 + 16*gamma - 1)  (1/4)*(6*gamma**2 - 20*gamma + 5)  gamma
 * ------------------------------------------------------------------------------------------
 *             | (1/4)*(-6*gamma**2 + 16*gamma - 1)  (1/4)*(6*gamma**2 - 20*gamma + 5)  gamma
 *
 * where gamma = -sqrt(2)*cos(atan(sqrt(2)/4)/3)/2 + sqrt(6)*sin(atan(sqrt(2)/4)/3)/2 + 1  ~
 * .435866521508459
 *
 * The stability function for this method is:
 * R(z) = (1.90128552647780115*z**2 + 2.46079651620301599*z - 8) /
 *        (0.662446064957040178*z**3 - 4.55951098972521484*z**2 + 10.460796516203016*z - 8)
 *
 * The method is L-stable:
 * lim R(z), z->oo = 0
 *
 * Notes: This method is derived in detail in: R. Alexander,
 * "Diagonally implicit Runge-Kutta Methods for Stiff ODEs", SIAM
 * J. Numer. Anal., 14(6), Dec. 1977, pg. 1006-1021.  Unlike BDF3,
 * this method is L-stable and so may be more suitable for "stiff"
 * problems.
 */
class LStableDirk3 : public TimeIntegrator
{
public:
  LStableDirk3(const InputParameters & parameters);
  virtual ~LStableDirk3();

  virtual int order() { return 3; }
  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  // Indicates the current stage.
  unsigned int _stage;

  // Store pointers to the various stage residuals
  NumericVector<Number> * _stage_residuals[3];

  // The parameter of the method, set at construction time and cannot be changed.
  const Real _gamma; // 0.4358665215084589

  // Butcher tableau "C" parameters derived from _gamma
  // 0.4358665215084589, 0.7179332607542295, 1.0000000000000000
  Real _c[3];

  // Butcher tableau "A" values derived from _gamma.  We only use the
  // lower triangle of this.
  // 0.4358665215084589
  // 0.2820667392457705,  0.4358665215084589
  // 1.2084966491760099, -0.6443631706844688, 0.4358665215084589
  Real _a[3][3];
};

#endif /* LSTABLEDIRK3_H */
