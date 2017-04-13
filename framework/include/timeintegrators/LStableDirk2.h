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

#ifndef LSTABLEDIRK2_H
#define LSTABLEDIRK2_H

#include "TimeIntegrator.h"

class LStableDirk2;

template <>
InputParameters validParams<LStableDirk2>();

/**
 * Second order diagonally implicit Runge Kutta method (Dirk) with two stages.
 *
 * The Butcher tableau for this method is:
 * alpha | alpha
 * 1     | 1-alpha alpha
 * ---------------------
 *       | 1-alpha alpha
 *
 * where alpha = 1 - sqrt(2)/2 ~ .29289
 *
 * The stability function for this method is:
 * R(z) = 4*(-z*(-sqrt(2) + 2) + z + 1) / (z**2*(-sqrt(2) + 2)**2 - 4*z*(-sqrt(2) + 2) + 4)
 *
 * The method is L-stable:
 * lim R(z), z->oo = 0
 *
 * Notes: This method is derived in detail in: R. Alexander,
 * "Diagonally implicit Runge-Kutta Methods for Stiff ODEs", SIAM
 * J. Numer. Anal., 14(6), Dec. 1977, pg. 1006-1021.  This method is
 * more expensive than Crank-Nicolson, but has the advantage of being
 * L-stable (the same type of stability as the implicit Euler method)
 * so may be more suitable for "stiff" problems.
 */
class LStableDirk2 : public TimeIntegrator
{
public:
  LStableDirk2(const InputParameters & parameters);
  virtual ~LStableDirk2();

  virtual int order() { return 2; }
  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  //! Indicates the current stage (1 or 2).
  unsigned int _stage;

  //! Buffer to store non-time residual from first stage solve.
  NumericVector<Number> & _residual_stage1;

  //! Buffer to store non-time residual from second stage solve
  NumericVector<Number> & _residual_stage2;

  // The parameter of the method, set at construction time and cannot be changed.
  const Real _alpha;
};

#endif /* LSTABLEDIRK2_H */
