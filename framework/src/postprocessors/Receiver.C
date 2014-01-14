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

#include "Receiver.h"

template<>
InputParameters validParams<Receiver>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<Real>("default", 0, "Default value");
  params.addParam<bool>("sum", false, "Whether or not to use the global sum as the value");
  return params;
}

Receiver::Receiver(const std::string & name, InputParameters params) :
    GeneralPostprocessor(name, params),
    _my_value(getPostprocessorValueByName(name)),
    _sum(getParam<bool>("sum"))
{
}

void
Receiver::initialSetup()
{
  // Set the default value
  _my_value = getParam<Real>("default");
}

Real
Receiver::getValue()
{
  if(_sum)
    gatherSum(_my_value);

  // Return the stored value (references stored value in getPostprocessorData)
  return _my_value;
}
