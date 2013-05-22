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

#ifndef IMPLICITEULER_H
#define IMPLICITEULER_H

#include "TimeIntegrator.h"

class ImplicitEuler;

template<>
InputParameters validParams<ImplicitEuler>();

/**
 * Implicit Euler's method
 */
class ImplicitEuler : public TimeIntegrator
{
public:
  ImplicitEuler(const std::string & name, InputParameters parameters);
  virtual ~ImplicitEuler();

  virtual int order() { return 1; }
  virtual void computeTimeDerivatives();
  virtual void postStep(NumericVector<Number> & residual);

protected:

};


#endif /* IMPLICITEULER_H */
