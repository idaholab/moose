//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NewmarkBeta_H
#define NewmarkBeta_H

#include "TimeIntegrator.h"

class NewmarkBeta;

template <>
InputParameters validParams<NewmarkBeta>();

/**
 * Implicit Euler's method
 */
class NewmarkBeta : public TimeIntegrator
{
public:
  NewmarkBeta(const InputParameters & parameters);
  virtual ~NewmarkBeta();

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  Real _beta;
  Real _gamma;
};

#endif /* NewmarkBeta_H */
