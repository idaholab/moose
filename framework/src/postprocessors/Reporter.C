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

#include "Reporter.h"

template<>
InputParameters validParams<Reporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

Reporter::Reporter(const std::string & name, InputParameters parameters):
  GeneralPostprocessor(name, parameters),
  _my_value(getPostprocessorValue( name ))
{}

Real
Reporter::getValue()
{
  return _my_value;
}
