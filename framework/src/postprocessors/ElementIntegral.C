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

#include "ElementIntegral.h"

template<>
InputParameters validParams<ElementIntegral>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

ElementIntegral::ElementIntegral(const std::string & name, InputParameters parameters)
  :ElementPostprocessor(name, parameters),
   _integral_value(0)
{}

void
ElementIntegral::initialize()
{
  _integral_value = 0;
}

void
ElementIntegral::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegral::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

Real
ElementIntegral::computeQpIntegral()
{
  return _u[_qp];
}
