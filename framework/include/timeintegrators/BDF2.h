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

#ifndef BDF2_H
#define BDF2_H

#include "TimeIntegrator.h"

class BDF2;

template<>
InputParameters validParams<BDF2>();

/**
 * BDF2 time integrator
 */
class BDF2 : public TimeIntegrator
{
public:
  BDF2(const std::string & name, InputParameters parameters);
  virtual ~BDF2();

  virtual int order() { return 2; }
  virtual void preStep();
  virtual void computeTimeDerivatives();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  std::vector<Real> & _weight;
};


#endif /* BDF2_H_ */
