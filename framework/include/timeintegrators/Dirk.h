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

#ifndef DIRK_H
#define DIRK_H

#include "TimeIntegrator.h"

class Dirk;

template<>
InputParameters validParams<Dirk>();

/**
 * Second order diagonally implicit Runge Kutta method (Dirk) with two stages.
 *
 * The Butcher tableau for this method is:
 * 1/3 | 1/3
 * 1   | 1/2 1/2
 * --------------
 *     | 3/4 1/4
 *
 * The stability function for this method is (the same as Crank-Nicolson):
 * R(z) = -(z + 2)/(z - 2)
 *
 * The method is A-stable, but not L-stable:
 * lim R(z), z->oo = -1
 *
 * Notes: This method has the same order of accuracy and stability
 * characteristics as the Crank-Nicolson method, but requires two
 * nonlinear solves (and is actually implemented with 3 nonlinear
 * solves for some reason) where Crank-Nicolson requires only one.
 * Therefore, it should generally be considered inferior to
 * Crank-Nicolson, and will likely be deprecated in MOOSE once better
 * methods become available.
 */
class Dirk : public TimeIntegrator
{
public:
  Dirk(const InputParameters & parameters);
  virtual ~Dirk();

  virtual int order() { return 2; }
  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:

  //! Indicates stage or, if _stage==3, the update step.
  unsigned int _stage;

  //! Order of the DIRK integrator. @note At the moment, only a second order DIRK is implemented
  unsigned int _order;

  //! Buffer to store non-time residual from first stage solve.
  NumericVector<Number> & _residual_stage1;

  //! Buffer to store non-time residual from second stage solve
  NumericVector<Number> & _residual_stage2;

  //! Buffer to store solution at beginning of time step
  NumericVector<Number> & _solution_start;
};


#endif /* DIRK_H */
