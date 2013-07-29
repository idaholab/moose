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

#ifndef EXPLICITEULER_H
#define EXPLICITEULER_H

#include "TimeIntegrator.h"

class ExplicitEuler;

template<>
InputParameters validParams<ExplicitEuler>();

/**
 * Explicit Euler time integrator
 */
class ExplicitEuler : public TimeIntegrator
{
public:
  ExplicitEuler(const std::string & name, InputParameters parameters);
  virtual ~ExplicitEuler();

  virtual int order() { return 1; }
  virtual void computeTimeDerivatives();
  virtual void postStep(NumericVector<Number> & residual);

protected:
};


#endif /* EXPLICITEULER_H */
