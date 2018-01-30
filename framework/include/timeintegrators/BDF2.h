//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BDF2_H
#define BDF2_H

#include "TimeIntegrator.h"

class BDF2;

template <>
InputParameters validParams<BDF2>();

/**
 * BDF2 time integrator
 */
class BDF2 : public TimeIntegrator
{
public:
  BDF2(const InputParameters & parameters);

  virtual int order() override { return 2; }
  virtual void preStep() override;
  virtual void computeTimeDerivatives() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

protected:
  std::vector<Real> & _weight;
};

#endif /* BDF2_H */
