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

#ifndef STEADYSTATE_H
#define STEADYSTATE_H

#include "TimeIntegrator.h"

class SteadyState;

template<>
InputParameters validParams<SteadyState>();

/**
 * "Time integrator" for steady state problems
 */
class SteadyState : public TimeIntegrator
{
public:
  SteadyState(const std::string & name, InputParameters parameters);
  virtual ~SteadyState();

  virtual int order() { return 0; }

  virtual void computeTimeDerivatives();
  virtual void postStep(NumericVector<Number> & residual);

protected:

};


#endif /* STEADYSTATE_H */
