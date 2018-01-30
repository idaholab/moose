//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  virtual void preSolve() override;
  virtual int order() override { return 2; }

  virtual void computeTimeDerivatives() override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  unsigned int _stage;

  /// Buffer to store non-time residual from the first stage.
  NumericVector<Number> & _residual_old;
};

#endif /* EXPLICITTVDRK2_H */
