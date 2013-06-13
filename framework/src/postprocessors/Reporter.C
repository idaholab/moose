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
  params.addParam<Real>("default", 0, "Default value");
  return params;
}

Reporter::Reporter(const std::string & name, InputParameters params) :
    GeneralPostprocessor(name, params),
    _my_value(getPostprocessorValue(name))
{
}

void
Reporter::initialSetup()
{
  // Set the default value
  _my_value = getParam<Real>("default");
}

Real
Reporter::getValue()
{
  // Return the stored value (references stored value in getPostprocessorData)
  return _my_value;
}
