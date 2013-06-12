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

#ifndef RUNGEKUTTA2_H
#define RUNGEKUTTA2_H

#include "TimeIntegrator.h"

class RungeKutta2;

template<>
InputParameters validParams<RungeKutta2>();

/**
 * RK-2
 */
class RungeKutta2 : public TimeIntegrator
{
public:
  RungeKutta2(const std::string & name, InputParameters parameters);
  virtual ~RungeKutta2();

  virtual int order() { return 2; }

  virtual void preSolve();
  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  NumericVector<Number> & _sln_half;
  NumericVector<Number> & _Re_half;

  unsigned int _stage;
};


#endif /* RUNGEKUTTA2_H */
