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

#ifndef ANATICALINDICATOR_H
#define ANATICALINDICATOR_H

#include "ElementIntegralIndicator.h"

class AnalyticalIndicator;

template<>
InputParameters validParams<AnalyticalIndicator>();

class AnalyticalIndicator :
  public ElementIntegralIndicator
{
public:
  AnalyticalIndicator(const std::string & name, InputParameters parameters);
  virtual ~AnalyticalIndicator(){};

protected:

  virtual Real computeQpIndicator();
};

#endif /* ANATICALINDICATOR_H */
