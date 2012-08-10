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

#include "AnalyticalIndicator.h"

template<>
InputParameters validParams<AnalyticalIndicator>()
{
  InputParameters params = validParams<ElementIntegralIndicator>();
  return params;
}


AnalyticalIndicator::AnalyticalIndicator(const std::string & name, InputParameters parameters) :
    ElementIntegralIndicator(name, parameters)
{
}

Real
AnalyticalIndicator::computeQpIndicator()
{
  return 1;
}

