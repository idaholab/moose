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

#ifndef EXPLICITRK2_H
#define EXPLICITRK2_H

#include "TimeIntegrator.h"

class ExplicitRK2;

template <>
InputParameters validParams<ExplicitRK2>();

/**
 * Base class for three different explicit second-order Runge-Kutta
 * time integration methods:
 * - Explicit midpoint method (ExplicitMidpoint.C)
 * - Heun's method (Heun.C)
 * - Ralston's method (Ralston.C)
 *
 * Each of these methods is characterized by the following generic
 * Butcher tableau:
 * 0   | 0
 * a   | a   0
 * ---------------------
 *     | b1  b2
 *
 * where a, b1, and b2 are constants that define the different
 * methods.
 *
 * The stability function for each of these methods is:
 * R(z) = z*(z + 2)/2 + 1
 *
 * The methods are all explicit, so they are neither A-stable nor L-stable,
 * lim R(z), z->oo = oo
 *
 * See also:
 *   https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
 *   https://en.wikipedia.org/wiki/Midpoint_method
 *   https://en.wikipedia.org/wiki/Heun%27s_method
 *
 * All three methods require two mass matrix (linear) system solves
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
class ExplicitRK2 : public TimeIntegrator
{
public:
  ExplicitRK2(const InputParameters & parameters);
  virtual ~ExplicitRK2();

  virtual void preSolve();
  virtual int order() { return 2; }

  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  unsigned int _stage;

  /// Buffer to store non-time residual from the first stage.
  NumericVector<Number> & _residual_old;

  /// The method coefficients.  See the table above for description.
  /// These are pure virtual in the base class, and must be overridden
  /// in subclasses to implement different schemes.
  virtual Real a() const = 0;
  virtual Real b1() const = 0;
  virtual Real b2() const = 0;
};

#endif /* EXPLICITRK2_H */
