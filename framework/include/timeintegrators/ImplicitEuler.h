//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IMPLICITEULER_H
#define IMPLICITEULER_H

#include "TimeIntegrator.h"

class ImplicitEuler;

template <>
InputParameters validParams<ImplicitEuler>();

/**
 * Implicit Euler's method
 */
class ImplicitEuler : public TimeIntegrator
{
public:
  ImplicitEuler(const InputParameters & parameters);
  virtual ~ImplicitEuler();

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
};

#endif /* IMPLICITEULER_H */
