//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CRANKNICOLSON_H
#define CRANKNICOLSON_H

#include "TimeIntegrator.h"

class CrankNicolson;

template <>
InputParameters validParams<CrankNicolson>();

/**
 * Crank-Nicolson time integrator.
 *
 * The scheme is defined as:
 *   \f$ \frac{du}{dt} = 1/2 * (F(U^{n+1}) + F(U^{n})) \f$,
 * but the form we are using it in is:
 *   \f$ 2 * \frac{du}{dt} = (F(U^{n+1}) + F(U^{n})) \f$.
 */
class CrankNicolson : public TimeIntegrator
{
public:
  CrankNicolson(const InputParameters & parameters);
  virtual ~CrankNicolson();

  virtual int order() { return 2; }
  virtual void computeTimeDerivatives();
  virtual void preSolve();
  virtual void postStep(NumericVector<Number> & residual);
  virtual void postSolve();

protected:
  NumericVector<Number> & _residual_old;
};

#endif /* CRANKNICOLSON_H */
