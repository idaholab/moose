//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXPLICITEULER_H
#define EXPLICITEULER_H

#include "TimeIntegrator.h"

class ExplicitEuler;

template <>
InputParameters validParams<ExplicitEuler>();

/**
 * Explicit Euler time integrator
 */
class ExplicitEuler : public TimeIntegrator
{
public:
  ExplicitEuler(const InputParameters & parameters);
  virtual ~ExplicitEuler();

  virtual void preSolve();
  virtual int order() { return 1; }
  virtual void computeTimeDerivatives();
  virtual void postStep(NumericVector<Number> & residual);

protected:
};

#endif /* EXPLICITEULER_H */
