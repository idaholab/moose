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
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}


AnalyticalIndicator::AnalyticalIndicator(const std::string & name, InputParameters parameters) :
    ElementIntegralIndicator(name, parameters),
    _func(getFunction("function"))
{
}

Real
AnalyticalIndicator::computeQpIntegral()
{
  Real diff = _u[_qp]-_func.value(_t, _q_point[_qp]);
  return diff*diff;
}

