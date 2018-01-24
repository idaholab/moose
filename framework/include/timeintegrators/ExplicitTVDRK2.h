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

#ifndef EXPLICITTVDRK2_H
#define EXPLICITTVDRK2_H

#include "TimeIntegrator.h"

class ExplicitTVDRK2;

template <>
InputParameters validParams<ExplicitTVDRK2>();

/**
 * Explicit TVD (total-variation-diminishing)
 * second-order Runge-Kutta time integration methods:
 *
 *
 *   Stage 1. R_{NL} = M*(U^{(1)}-U^n)/dt - F(t^n,U^n)
 *
 *            t^{(1)} = t^{n} + dt = t^{n+1}
 *
 *   Stage 2. R_{NL} = M*(2*U^{(2)}-U^{(1)}-U^n)/(2*dt) - (1/2)*F(t^{(1)},U^{(1)})
 *
 *            U^{n+1} = U^{(2)}
 *
 *   Reference:
 *   Gottlieb, S., & Shu, C. W. (1998).
 *   Total variation diminishing Runge-Kutta schemes.
 *   Mathematics of computation of the American Mathematical Society,
 *   67(221), 73-85.
 *
 * The method requires two mass matrix (linear) system solves
 * per timestep. Although strictly speaking these are "two stage"
 * methods, we treat the "update" step as a third stage, since in
 * finite element analysis the update step requires a mass matrix
 * solve.
 *
 * IMPORTANT: To use the explicit TimeIntegrators derived from this
 * method, you must generally add "implicit=false" to the Kernels,
 * Materials, etc. used in your simulation, so that MOOSE evaluates
 * them correctly!  An important exception are TimeDerivative kernels,
 * which should never be marked "implicit=false".
 */
class ExplicitTVDRK2 : public TimeIntegrator
{
public:
  ExplicitTVDRK2(const InputParameters & parameters);
  virtual ~ExplicitTVDRK2();

  virtual void preSolve();
  virtual int order() { return 2; }

  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  unsigned int _stage;

  /// Buffer to store non-time residual from the first stage.
  NumericVector<Number> & _residual_old;
};

#endif /* EXPLICITTVDRK2_H */
